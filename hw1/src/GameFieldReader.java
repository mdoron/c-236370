import java.io.*;
import java.util.*;

public class GameFieldReader{
	
	String filename;
	int sizeX;
	int sizeY;
	
	public GameFieldReader(String filename, int sizeX,int sizeY){
		this.filename=filename;
		this.sizeX=sizeX;
		this.sizeY=sizeY;
	}
	
	public boolean[][] parseData() throws Exception{
	  BufferedReader br=null;
	  ArrayList<PatternBlock> patternBlocks=new ArrayList<PatternBlock>();
	  try{
		br=new BufferedReader(new FileReader(filename));	
		String line;
		

		String coords=null;
		ArrayList<String> pattern=null;
		
		while((line=br.readLine())!=null){

			line=line.trim();
			// check the first symbol
			if (line.length()==0||
					(line.charAt(0)=='#' && line.length()==1)) {
				continue;
			}
			
			
			
			if (line.startsWith("#P")){
				// found (new) pattern block
				if (coords!=null){
					// create block
					patternBlocks.add(new PatternBlock(coords,pattern));
				}
				coords=line;
				pattern=new ArrayList<String>();
				continue;
			}
			
			if (line.charAt(0)!='#' && coords!=null){
				pattern.add(line);
			}
		}
		if (coords!=null){
			// create block
			patternBlocks.add(new PatternBlock(coords,pattern));
		}
	  }catch(IOException ex){
		System.err.println("Exception occured while reading/opening a file "+filename+":"+ex);
		ex.printStackTrace();
		System.exit(-1);
	  }
	  br.close();
	  return buildGameField(patternBlocks);
	}
	
	private boolean[][] buildGameField(ArrayList<PatternBlock> patternBlocks) throws Exception{
		boolean[][] result=new boolean[sizeY][];
		for (int i=0;i<sizeY;i++){
			result[i]=new boolean[sizeX];
		}

		for(PatternBlock block: patternBlocks){
			int startX=block.getX()+sizeX/2;
			int startY=block.getY()+sizeY/2;
			
			if (startX+block.getMaxX()>sizeX||startX<0){
				throw new Exception("Field is too small in X direction");
			}
			
			if (startY+block.getMaxY()>sizeY||startY<0){
				throw new Exception("Field is too small in Y direction");
			}
			
			for(boolean[] patternLine:block.getPattern()){
				int i=0;
				for(boolean dot:patternLine){
					result[startY][startX+(i++)]=dot;
				}
				startY++;
			}
		}
		
		return result;
	}
	
//	public static void main(String[] args) {
//		try{
//			GameFieldReader g=new GameFieldReader(args[0],Integer.parseInt(args[1]),
//				Integer.parseInt(args[2]));
//			
//			boolean[][] arr=g.parseData();
//			Ex1.printArray(arr);
//			
//		}catch(Exception ex){
//			System.err.println(ex);
//			ex.printStackTrace();
//		}
//		
//		
//	}

}
class PatternBlock{
	// left upper conner coords
	int x;
	int y;
	int maxX=0;
	int maxY=0;

	ArrayList<boolean[]> pattern=new ArrayList<boolean[]>();
	
	public PatternBlock(String coords, ArrayList<String> rawPattern) throws Exception{
		parsePattern(coords,rawPattern);
	}
	
	
	private void parsePattern(String coords, ArrayList<String> rawPattern) throws Exception{
		StringTokenizer st=new StringTokenizer(coords);
		if (st.countTokens()!=3){
			throw new Exception("Wrong file format -> cannot parse pattern coordinates");
		}
		// skip first element
		st.nextToken();
		x=Integer.parseInt(st.nextToken());
		y=Integer.parseInt(st.nextToken());
		
		for (String line:rawPattern){
			int length=line.length();
			maxX=(maxX>length?maxX:length);
			
			boolean[] patternLine=new boolean[length];
			int counter=0;
			for (char dot: line.toCharArray()){
				switch (dot){
				case '*': patternLine[counter++]=true;break;
				case '.': patternLine[counter++]=false;break;
				default: throw new Exception("Wrong pattern format - cannot parse");
				}
			}
			
			pattern.add(patternLine);
		}
		maxY=pattern.size();
	}


	public ArrayList<boolean[]> getPattern() {
		return pattern;
	}


	public int getX() {
		return x;
	}


	public int getY() {
		return y;
	}


	public int getMaxX() {
		return maxX;
	}


	public int getMaxY() {
		return maxY;
	}
}
