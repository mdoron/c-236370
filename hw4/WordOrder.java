import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.input.KeyValueTextInputFormat;
import org.apache.hadoop.fs.FileSystem;

public class WordOrder {
	// User local temp folder
	private static final Path TEMP_PATH = new Path("temp");

	public static class TokenizerMapper	extends Mapper<LongWritable, Text, Text, IntWritable> {
		private final static IntWritable one = new IntWritable(1);
		private final Text word = new Text();

		public void map(LongWritable key, Text value, Context context)
				throws IOException, InterruptedException {
			StringTokenizer st = new StringTokenizer(value.toString());
			while (st.hasMoreTokens()) {
				word.set(st.nextToken());
				context.write(word, one);
			}
		}
	}

	public static class IntSumReducer extends Reducer<Text,IntWritable,Text,IntWritable> {
		private final IntWritable result = new IntWritable();

		public void reduce(Text key, Iterable<IntWritable> values, Context context)
				throws IOException, InterruptedException {
			int sum = 0;
			for (IntWritable val : values) {
				sum += val.get();
			}
			result.set(sum);
			context.write(key, result);
		}
	}

	public static class SwapMapper extends Mapper<Text, Text, IntWritable, Text> {
		private final IntWritable count = new IntWritable();

		public void map(Text key, Text value, Context context)
				throws IOException, InterruptedException {
			count.set(Integer.parseInt(value.toString()));
			context.write(count, key);
		}
	}

	public static class OutputReducer extends Reducer<IntWritable, Text, Text, NullWritable> {
		private final static NullWritable nothing = NullWritable.get();

		public void reduce(IntWritable key, Iterable<Text> values, Context context)
				throws IOException, InterruptedException {
			for (Text val : values) {
				context.write(val, nothing);
			}
		}
	}

	public static void main(String[] args) throws Exception {
		Configuration conf = new Configuration();
		FileSystem fs = FileSystem.get(conf);

		// Just to be safe: clean temporary files before we begin
		fs.delete(TEMP_PATH, true);

		/* We chain the two Mapreduce phases using a temporary directory
		from which the first phase writes to, and the second reads from */
		
		// Setup first MapReduce phase
		Job job1 = Job.getInstance(conf, "WordOrder-first");
		job1.setJarByClass(WordOrder.class);
		job1.setMapperClass(TokenizerMapper.class);
		job1.setReducerClass(IntSumReducer.class);
		job1.setMapOutputKeyClass(Text.class);
		job1.setMapOutputValueClass(IntWritable.class);
		job1.setOutputKeyClass(Text.class);
		job1.setOutputValueClass(IntWritable.class);
		FileInputFormat.addInputPath(job1, new Path(args[0]));
		FileOutputFormat.setOutputPath(job1, TEMP_PATH);
		
		boolean status1 = job1.waitForCompletion(true);
		if(!status1) {
			System.exit(1);
		}
		
		// Setup second MapReduce phase
		Job job2 = Job.getInstance(conf, "WordOrder-second");
		job2.setJarByClass(WordOrder.class);
		job2.setMapperClass(SwapMapper.class);
		job2.setReducerClass(OutputReducer.class);
		job2.setMapOutputKeyClass(IntWritable.class);
		job2.setMapOutputValueClass(Text.class);
		job2.setOutputKeyClass(Text.class);
		job2.setOutputValueClass(NullWritable.class);
		job2.setInputFormatClass(KeyValueTextInputFormat.class);
		FileInputFormat.addInputPath(job2, TEMP_PATH);
		FileOutputFormat.setOutputPath(job2, new Path(args[1]));
		
		boolean status2 = job2.waitForCompletion(true);
		
		// Clean temporary files from the first MapReduce phase
		fs.delete(TEMP_PATH, true);

		if (!status2) System.exit(1);
	}
}