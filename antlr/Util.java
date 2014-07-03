import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;

import org.antlr.v4.runtime.ParserRuleContext;
import org.antlr.v4.runtime.TokenStream;

class Debug {
	static int debug = 1;
	public void println(String s) {
		if(debug == 1) {
			System.out.flush();		// For ordering b/w stderr and stdout
			System.err.println(s);
		}
	}
	
	public void print(String s) {
		if(debug == 1) {
			System.out.flush();
			System.err.print(s);
		}
	}
	
	// ParserRuleContext.getText() seems to ignore the hidden tokens
	// Use this for printing contexts instead
	public String btrText(ParserRuleContext ctx, TokenStream tokens) {
		int startIndex = ctx.getStart().getTokenIndex();
		int stopIndex = ctx.getStop().getTokenIndex();

		String ret = "";
		for(int i = startIndex; i <= stopIndex; i ++) {
			ret = ret + tokens.get(i).getText() + "";	
		}
		
		return ret;
	}
	
	// Get a String representation of the input code
	public String getCode(String gotoFilePath) throws FileNotFoundException {
		Scanner c = new Scanner(new File(gotoFilePath));
		String res = "";
		while(c.hasNext()) {
			res += c.nextLine();
			res += "\n";
		}
		c.close();
		return res;
	}
	
	public String removeDoubleNewlines(String code) {
		Scanner c = new Scanner(code);
		String cleanerCode = "";
		
		boolean lastBlank = false;
		while(c.hasNext()) {
			String nowAdding = c.nextLine();
			if(nowAdding.matches("[' ', '\t']*")) {
				if(lastBlank) {
					continue;
				} else {
					cleanerCode = cleanerCode + nowAdding + "\n";
					lastBlank = true;
				}
			} else {
				cleanerCode = cleanerCode + nowAdding + "\n";
				lastBlank = false;
			}
		}
		
		c.close();
		return cleanerCode;
	}
}

class VariableDecl {
	String type;
	String name;
	String value;
	
	public VariableDecl(String type, String name, String value) {
		this.type = type;
		this.name = name;
		this.value = value;
	}
	
	public String toString() {
		if(value.contentEquals("")) {
			return type + " " + name + ";";
		} else {
			return type + " " + name + " = " + value + ";";
		}
	}
	
	public String arrayDecl() {
		return type + " " + name + "[BATCH_SIZE]" + ";";
	}
	
	@Override
	public boolean equals(Object o) {
		VariableDecl ov = (VariableDecl) o;
		
		// We store local variable names along with the *s. For example, 
		// int **baz = 1; is stored as (int, **baz, 1). When searching for 
		// Object x in a list, x.equals(o) is called for each o in the list.
		
		String ovRealName = ov.name.replaceAll("[*]+", "");
		// System.err.println("\tComparing " + this.name + " with localVar " + ovRealName);
		if(ovRealName.contentEquals(this.name)) {
			return true;
		}
		return false;
	}
}