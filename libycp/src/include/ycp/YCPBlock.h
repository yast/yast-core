/*---------------------------------------------------------------------\
|                                                                      |  
|                      __   __    ____ _____ ____                      |  
|                      \ \ / /_ _/ ___|_   _|___ \                     |  
|                       \ V / _` \___ \ | |   __) |                    |  
|                        | | (_| |___) || |  / __/                     |  
|                        |_|\__,_|____/ |_| |_____|                    |  
|                                                                      |  
|                               core system                            | 
|                                                        (C) SuSE GmbH |  
\----------------------------------------------------------------------/ 

   File:       YCPBlock.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPBlock_h
#define YCPBlock_h


#include <ycp/YCPValue.h>
#include <ycp/YCPStatement.h>


/**
 * @short Block Type
 * Defines constants for the Block types. The Block type specifies the class
 * the YCPBlockRep object belongs to.
 */
enum YCPBlockType
{
  YB_PLAIN   = 0,
  YB_WHILE   = 1,
  YB_DOWHILE = 2
};


/**
 * @short A YCP communication block
 * A variable of this type represents a parsed YCP protocol block. It
 * is a base class for a couple of different block types,
 * which differ in the way break, continue and return are handled and
 * which implement control structures like while, do..while, for i in ...
 * It can be instantiated and represents a block without control structures.
 */
class YCPBlockRep : public YCPValueRep
{
    /**
     * name of module
     */
    const string module_name;

    /**
     * the statements of this block
     */
    vector<YCPStatement> statements;

    /**
     * Name of the file this block is defined in.
     */
    string file_name;

protected:
    friend class YCPBlock;

    /**
     * Creates a new (and empty) YCP block.
     */
    YCPBlockRep();
    YCPBlockRep(const string name);
    YCPBlockRep(const string name, const vector<YCPStatement>& statements);

    /**
     * Cleans up
     */
    ~YCPBlockRep() {}

    /**
     * Locally compares the statements of two blocks for equality,
     * greaterness or smallerness.
     * @param b block to compare against
     * @return YO_LESS,    if this is smaller than b,
     *         YO_EQUAL,   if this is equal to b,
     *         YO_GREATER, if this is greater than b
     */ 
    YCPOrder compare_statements(const YCPBlock &b) const;
    
public:
    /**
     * Returns the type of the block. If you just want to check, whether it is
     * legal to cast an object of the YCPBlockRep to a certain more specific
     * object, you should use one of the is... methods.
     */
    virtual YCPBlockType blocktype() const;
    
    /**
     * Returns true, if this object is of the type YCPWhileBlockRep.
     */
    bool isWhileBlock() const;
    
    /**
     * Returns true, if this object is of the type YCPDoWhileBlockRep.
     */
    bool isDoWhileBlock() const;

    /**
     * Returns true, if this object is of the type YCPDoWhileBlockRep
     * or YCPWhileBlockRep.
     */
    bool isLoopBlock() const;

   /**
    * Casts this element into a pointer of type const YCPWhileBlock
    */
    YCPWhileBlock asWhileBlock() const; 

   /**
    * Casts this element into a pointer of type const YCPDoWhileBlock
    */
    YCPDoWhileBlock asDoWhileBlock() const; 

    /**
     * Appends a statement to the block.
     * @param s statement to add
     */
    void add(const YCPStatement& s);

    /**
     * Returns one of the statements.
     * @param index number of the statement in the block (0 is the first).
     */
    YCPStatement statement(int index) const;

    /**
     * @return the number of statements in the block
     */
    int size() const;

    /**
     * Compares two blocks for equality, greaterness or smallerness.
     * @param b block to compare against
     * Comparison is done as follows:
     *   Plain blocks < while blocks < do while blocks.
     *   Same block types are compared in conditions first
     *   and in statements thereafter if conditions are equal.
     *   For conditions and statements a smaller size means less.
     *   In case of equal sizes the components of statements and
     *   conditions are compared with respect to their types. 
     * @return YO_LESS,    if this is smaller than b,
     *         YO_EQUAL,   if this is equal to b,
     *         YO_GREATER, if this is greater than b
     */ 
    YCPOrder compare(const YCPBlock &b) const;

    /**
     * Returns an ASCII representation of the list.
     * Lists are denoted by comma separated values enclosed
     * by square brackets.
     */
    string toString() const;

    /**
     * Returns YT_BLOCK. See @ref YCPValueRep#valuetype.
     */
    YCPValueType valuetype() const;

    /**
     * Evaluates the block within an interpreter.
     */
    YCPValue evaluate(YCPBasicInterpreter *interpreter, 
		      bool& do_break, bool& do_continue, bool& do_return) const;


    /**
     * Set file_name. Used for quoted blocks in defines
     * @param fn new file name
     */
    void setFileName (const char*fn)
            { if(NULL!=fn) file_name = fn; }

protected:
    /**
     * Defines an initial action that should be executed when
     * the block is entered.
     * The default implementation does nothing and returns true.
     * @param interpreter Used to evaluate conditions
     * @return false, if the block should be left afterwards.
     */
    virtual bool handleBlockInit(YCPBasicInterpreter *interpreter) const;

    /**
     * Defines an action that should be executed just before
     * the first statement of the block is executed and each
     * time, the program counter is set to the block beginning.
     * The default implementation does nothing and returns true.
     * @param interpreter Used to evaluate conditions
     * @return false, if the block should be left afterwards.
     */
    virtual bool handleBlockHead(YCPBasicInterpreter *interpreter) const;

    /**
     * Defines an action that should be executed just after
     * the last statement of the block is executed and each
     * time, the continue statement is handled by this block.
     * The default implementation does nothing and return false.
     * @param interpreter Used to evaluate conditions
     * @return false, if the block should be left afterwards.
     */
    virtual bool handleBlockEnd(YCPBasicInterpreter *interpreter) const;

    /**
     * Defines, how this block handles the break statement.
     * The default implementation sets do_break to true and
     * returns false.
     * @return false, if the block should be left due to the break statement.
     */
    virtual void handleBreak(bool& do_break) const;

    /**
     * Defines, how this block handles the break statement.
     * The default implementation sets do_continue to true
     * and return false.
     * @return false, if the block should be left due to the continue statement
     */
    virtual bool handleContinue(bool& do_continue, int& program_counter) const;

    /**
     * Helper function that evaluates evaluation and if/then statements.
     */
    YCPValue evaluateStatement(const YCPStatement& statement,
			       YCPBasicInterpreter *interpreter, 
			       bool& do_break, bool& do_continue, bool& do_return) const;


    YCPBlockRep(const vector<YCPStatement>&);

public:
    const vector<YCPStatement>& getStatements() const { return statements; }
};

/**
 * @short Wrapper for YCPBlockRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPBlockRep
 * with the arrow operator. See @ref YCPBlockRep.
 */
class YCPBlock : public YCPValue
{
    DEF_COMMON(Block, Value);
public:
    YCPBlock() : YCPValue(new YCPBlockRep(string(""))) {}
    YCPBlock(const string name) : YCPValue(new YCPBlockRep(name)) {}
    YCPBlock(const string name, const vector<YCPStatement>& statements) : YCPValue(new YCPBlockRep(name, statements)) {}
};

#endif   // YCPBlock_h
