#!/usr/bin/python

import os
import sys
import unittest
import logging
import argparse
import shutil
import subprocess
import random
import tempfile


def bad_chars():
    # make a list characters we need to ignore.
    z = range(14, 256) # skip special characters 0..13
    good = range(ord('a'), ord('z')+1) + range(ord('A'), ord('Z')+1) + range(ord('0'), ord('9')+1)
    for c in good:
        z.remove(c)
    return map(chr, z)

class lcg(object):
    def __init__( self, seed=1 ):
        self.state = seed

    def random(self):
        self.state = (self.state * 1103515245 + 12345) & 0x7FFFFFFF
        return self.state


def hdfs_ls(path, recursive=False):
    flags = ''
    if recursive:
        flags = '-R'
    cmd = 'hadoop fs -ls %s %s' % (flags, path)
    d = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=open(os.devnull, 'wb'), shell=True).stdout.read()
    # doesn't work for files with spaces
    # if no -R, a first line with 'Found XX items' is printed. ignore it.
    files = [line.split()[-1] for line in d.splitlines() if not line.startswith('Found ')]
    return files

def hdfs_rmdir(paths):
    if type(paths) is list:
        path = ' '.join(paths)
    cmd = 'hadoop fs -rm -r -f %s' % path
    subprocess.check_call(cmd, shell=True, stderr=open(os.devnull, 'wb'), stdout=open(os.devnull, 'wb'))

def hdfs_mkdir(path):
    cmd = 'hadoop fs -mkdir %s' % path
    subprocess.check_call(cmd, shell=True, stderr=open(os.devnull, 'wb'))

def hdfs_put(path, text):
    f = tempfile.NamedTemporaryFile(delete=False)
    fname = f.name
    f.write(text)
    f.close()
    cmd = 'hadoop fs -put %s %s' % (to_windows83(fname), path)
    subprocess.check_call(cmd, shell=True, stderr=open(os.devnull, 'wb'))
    os.unlink(fname)

def hdfs_get(path):
    cmd = 'hadoop fs -cat %s' % path
    return subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=open(os.devnull, 'wb'), shell=True).stdout.read()

# windows 8.3 path utilities
if os.name == 'nt':
    win83_offset = 34
    win83_len = 13
    # find offset in dir /X /A command
    lines = os.popen('dir /X /A').read().splitlines()[5:]
    for line in lines:
        line = line.rstrip()
        if line.endswith(' ..'):
            win83_offset = len(line) - 2 - win83_len
            #print 'new win83 offset =', win83_offset
            break
    else:
        print >> sys.stderr, 'WARN: Could not find offset for windows 8.3'

def to_windows83(path):
    if os.name != 'nt':
        return path
    # Hack to get long paths to work on windows.
    d = os.path.dirname(path)
    if d == path:
        return path
    base = os.path.basename(path)
    files = os.popen('dir /X /A "%s"' % d).read().splitlines()[5:]
    files = dict([
        (f[win83_offset + win83_len:].lower(),
            f[win83_offset:win83_offset+win83_len].rstrip())
        for f in files])
    w83 = files[base.lower()].strip()
    if not w83:
        w83 = base
    return os.path.join(to_windows83(d), w83)


class Tests(unittest.TestCase):
    ROOT = '.'
    DOCS = 'docs_blabla3tqafa'
    OUTPUT = 'output_blabla3tqafa'
    BAD_CHARS = bad_chars()

    @classmethod
    def setUpClass(cls):
        logging.info("Building detector")
        cls.build()
        logging.info("Build OK")

    def setUp(self):
        self.clean()
        self.files_before = hdfs_ls(self.ROOT, True)
        hdfs_mkdir(self.DOCS)

    def tearDown(self):
        self.clean()
        new_files = set(hdfs_ls(self.ROOT, True)) - set(self.files_before)
        self.assertEqual(new_files, set())

    @classmethod
    def build(cls):
        src = os.path.join(cls.ROOT, 'Detector.java')
        cmd = 'hadoop com.sun.tools.javac.Main %s' % src
        if os.system(cmd) != 0:
            raise RuntimeError("Build failed: " + cmd)
        
        dclass = os.path.join(cls.ROOT, '*.class')
        cmd = 'jar cf detector.jar %s' % dclass
        if os.system(cmd) != 0:
            raise RuntimeError("Build failed: " + cmd)
    
    @classmethod
    def detect(cls, n, k):
        cmd = 'hadoop jar detector.jar Detector %d %d %s %s' %\
                (n, k, cls.DOCS, cls.OUTPUT)
        subprocess.check_call(cmd, shell=True,
            stderr=open(os.devnull, 'wb'), stdout=open(os.devnull, 'wb'))
        out = hdfs_get('%s/%s' % (cls.OUTPUT, 'part-r-00000'))
        d = {}
        for l in out.splitlines():
            files = tuple(l.split('\t')[:2])
            count = int(l.split('\t')[2])
            if files in d:
                raise RuntimeError("Pair of files appeared more than once: " + str(files))
            d[files] = count
        return d

    @classmethod
    def add_file(self, filename, text):
        hdfs_put('%s/%s' % (self.DOCS, filename), text)

    @classmethod
    def clean(self):
        hdfs_rmdir([self.DOCS, self.OUTPUT])

    def test_simple(self):
        self.add_file('f1', 'hello hello hi')
        self.add_file('f2', 'hi hello lol')
        results = self.detect(100, 1)
        self.assertEqual(results[('f1', 'f2')], 5)
        self.assertEqual(len(results), 1)
    
    def test_lower_upper_case(self):
        self.add_file('f1', 'hello cat')
        self.add_file('f2', 'hi Cat')
        results = self.detect(100, 1)
        self.assertEqual(results[('f1', 'f2')], 2)
        self.assertEqual(len(results), 1)
    
    def test_sorted_names(self):
        self.add_file('f2', 'hi')
        self.add_file('f1', 'hi')
        self.add_file('f4', 'hi')
        self.add_file('f3', 'hi')
        results = self.detect(100, 1)
        self.assertTrue(('f1', 'f2') in results)
        self.assertTrue(('f1', 'f3') in results)
        self.assertTrue(('f1', 'f4') in results)
        self.assertTrue(('f2', 'f3') in results)
        self.assertTrue(('f2', 'f4') in results)
        self.assertTrue(('f3', 'f4') in results)
        self.assertEqual(len(results), 6)
    
    def test_n(self):
        self.add_file('f1', 'hello hello hi')
        self.add_file('f2', 'hi hello hello hi hello lol')
        results = self.detect(1, 1)
        self.assertEqual(results[('f1', 'f2')], 5)
        self.assertEqual(len(results), 1)
    
    def test_k(self):
        self.add_file('f1', 'hello hello hi hiush')
        self.add_file('f2', 'hi hello lol')
        self.add_file('f3', 'hi hello hiush')
        
        self.add_file('f4', 'hi1')
        self.add_file('f5', 'hi1 hi1 hi1 hi1 hi1')
        
        self.add_file('f6', 'hi2')
        self.add_file('f7', 'hi2 hi2 hi2 hi2 hi2 hi2')
        
        self.add_file('f8', 'hi3 hi3')
        self.add_file('f9', 'hi3 hi3 hi3 hi3 hi3 hi3')
        
        results = self.detect(100, 7)
        self.assertEqual(results[('f1', 'f3')], 7)
        self.assertEqual(results[('f6', 'f7')], 7)
        self.assertEqual(results[('f8', 'f9')], 8)
        self.assertEqual(len(results), 3)
    
    def test_bad_chars(self):
        # repeatable random
        rand = lcg(123)
        # text file with random bad characters and many instances of 'a'
        d = ''
        for _ in range(1000):
            d += 'a'
            for _ in range(1, 1 + rand.random() % 10):
                d += self.BAD_CHARS[rand.random() % len(self.BAD_CHARS)]
        self.add_file('f1', 'a')
        self.add_file('f2', d)
        results = self.detect(100, 1)
        self.assertEqual(results[('f1', 'f2')], 1001)
        self.assertEqual(len(results), 1)

if __name__ == '__main__':
    logging.getLogger().setLevel(logging.INFO)
    # under windows, we have to use windows 8.3 paths.
    if os.name == 'nt':
        os.chdir(to_windows83(os.getcwd()))
    # various hacks for python < 2.7
    if sys.version < '2.7':
        # no setUpClass() in old python..
        Tests.setUpClass()
    sys.argv.insert(1, '-v')
    unittest.main()
