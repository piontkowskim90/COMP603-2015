/* Mark Piontkowski
= Brainfuck

If you have gcc:

----
g++ -o brainfuck.exe brainfuck.cpp
brainfuck.exe helloworld.bf
----
*/

#include <vector>
#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

/**
 * Primitive Brainfuck commands
 */
typedef enum {
    INCREMENT, // +
    DECREMENT, // -
    SHIFT_LEFT, // <
    SHIFT_RIGHT, // >
    INPUT, // ,
    OUTPUT, // .
    ZERO //0
} Command;

// Forward references. Silly C++!
class CommandNode;
class Loop;
class Program;
class Container;
class Printer;
class Interpreter;

/**
 * Visits?!? Well, that'd indicate visitors!
 * A visitor is an interface that allows you to walk through a tree and do stuff.
 */
class Visitor {
    public:
        virtual void visit(const CommandNode * leaf) = 0;
        virtual void visit(const Loop * loop) = 0;
        virtual void visit(const Program * program) = 0;
};

/**
 * The Node class (like a Java abstract class) accepts visitors, but since it's pure virtual, we can't use it directly.
 */
class Node {
    public:
        virtual void accept (Visitor *v) = 0;
};

/**
 * CommandNode publicly extends Node to accept visitors.
 * CommandNode represents a leaf node with a primitive Brainfuck command in it.
 */
class CommandNode : public Node {
    public:
        Command command;
        int count;
        CommandNode(char c, int i){
            count = i;
            switch(c) {
                case '+': command = INCREMENT; break;
                case '-': command = DECREMENT; break;
                case '<': command = SHIFT_LEFT; break;
                case '>': command = SHIFT_RIGHT; break;
                case ',': command = INPUT; break;
                case '.': command = OUTPUT; break;
                case '0': command = ZERO; break;
            }
        }
        void accept (Visitor * v) {
            v->visit(this);
        }
};

class Container : public Node{
public:
    vector<Node*> children;
    virtual void accept(Visitor * v) = 0;
};

/**
 * Loop publicly extends Node to accept visitors.
 * Loop represents a loop in Brainfuck.
 */
class Loop : public Container {
    public:
        void accept (Visitor * v) {
            v->visit(this);
        }
};

/**
 * Program is the root of a Brainfuck program abstract syntax tree.
 * Because Brainfuck is so primitive, the parse tree is the abstract syntax tree.
 */
class Program : public Container {
    public:
        void accept (Visitor * v) {
            v->visit(this);
        }
};

/**
 * Read in the file by recursive descent.
 * Modify as necessary and add whatever functions you need to get things done.
 */
void parse(fstream & file, Container * contain) {
    int count;
    char c;
    Loop *looper;

	while (file >> c){
	count = 0; //Reset count upon new char
		if (c == '['){
            looper = new Loop();
           	parse(file, looper); //Recursively call parse on new loop
 /*           if(looper->children.size() == 1){
					Node * test = looper->children[0];
					//How to reach commandnode from node?
					if(test->command == INCREMENT || test->command == DECREMENT){
						contain->children.push_back(new CommandNode('0',1)); //Add ZERO to program
					}
					else{contain->children.push_back(looper); }//Add sub-loop to program
			}
			else{contain->children.push_back(looper);} //Add sub-loop to program
*/			contain->children.push_back(looper);

		}
		if (c == ']'){	return; }//close loop container
		else{//Must be Command
			count = 1; //at least 1 char
			while(c == file.peek()){
				file >> c;
				count++;
			}
			contain->children.push_back(new CommandNode(c,count)); // Add leaf to program
		}
	}
}

/**
 * A printer for Brainfuck abstract syntax trees.
 * As a visitor, it will just print out the commands as is.
 * For Loops and the root Program node, it walks trough all the children.
 */
class Printer : public Visitor {
    public:
        void visit(const CommandNode * leaf) {
            switch (leaf->command) {
                case INCREMENT:   cout << leaf->count << '+'; break;
                case DECREMENT:   cout << leaf->count << '-'; break;
                case SHIFT_LEFT:  cout << leaf->count << '<'; break;
                case SHIFT_RIGHT: cout << leaf->count << '>'; break;
                case INPUT:       cout << leaf->count << ','; break;
                case OUTPUT:      cout << leaf->count << '.'; break;
                case ZERO:		  cout << leaf->count << 'Z'; break;
            }
        }
        void visit(const Loop * loop) {
            cout << '[';
            for (vector<Node*>::const_iterator it = loop->children.begin(); it != loop->children.end(); ++it) {
                (*it)->accept(this);
            }
            cout << ']';
        }
        void visit(const Program * program) {
            for (vector<Node*>::const_iterator it = program->children.begin(); it != program->children.end(); ++it) {
                (*it)->accept(this);
            }
            cout << '\n';
        }
};

class Interpreter : public Visitor {
    public:
    	char memory[30000];
    	char* ptr;
        void visit(const CommandNode * leaf) {
            int count = leaf->count;
            switch (leaf->command) {
                case INCREMENT:
                		while(count > 0){
            				++(*ptr);
            				count--; }
                		break;
                case DECREMENT:
                		while(count > 0){
                			--(*ptr);
                			count--; }
                		break;
                case SHIFT_LEFT:
                		while(count > 0){
                			--ptr;
                			count--; }
                		break;
                case SHIFT_RIGHT:
                		while(count > 0){
                			++ptr;
                			count--; }
                		break;
                case INPUT:
                		while(count > 0){
                			*ptr=getchar();
                			count--; }
                		break;
                case OUTPUT:
                		while(count > 0){
                			putchar(*ptr);
                			count--; }
                		break;
                case ZERO:
				       *ptr = 0;
                		break;
            }
        }
        void visit(const Loop * loop) {
			while(*ptr > 0){
            	for (vector<Node*>::const_iterator it = loop->children.begin(); it != loop->children.end(); ++it) {
                	(*it)->accept(this);
            	}
			}
        }
        void visit(const Program * program) {
			memset(memory, 0, 30000);
			ptr = memory;
            for (vector<Node*>::const_iterator it = program->children.begin(); it != program->children.end(); ++it) {
                (*it)->accept(this);
            }
            cout << '\n';
        }
};

int main(int argc, char *argv[]) {
    fstream file;
    Program program;
    Printer printer;
    Interpreter interpret;

    if (argc == 1) {
        cout << argv[0] << ": No input files." << endl;
    } else if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            file.open(argv[i], fstream::in);
            parse(file, & program);
            program.accept(&printer);
      		program.accept(&interpret);  //Use interpreter as a visitor
            file.close();
        }
    }
}
