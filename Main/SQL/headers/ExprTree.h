
#ifndef SQL_EXPRESSIONS
#define SQL_EXPRESSIONS

#include "MyDB_AttType.h"
#include "MyDb_Catalog.h"
#include <string>
#include <vector>

// create a smart pointer for database tables
using namespace std;
class ExprTree;
typedef shared_ptr <ExprTree> ExprTreePtr;

// this class encapsules a parsed SQL expression (such as "this.that > 34.5 AND 4 = 5")

// class ExprTree is a pure virtual class... the various classes that implement it are below
class ExprTree {

public:
	virtual bool validate_tree(MyDB_CatalogPtr)=0;
	virtual string type_check(MyDB_CatalogPtr)=0;
	virtual bool in_group_clause(MyDB_CatalogPtr)=0;
	virtual string get_tpye()=0;
	virtual string toString () = 0;
	virtual ~ExprTree () {}
};

class BoolLiteral : public ExprTree {

private:
	bool myVal;
public:
	
	BoolLiteral (bool fromMe) {
		myVal = fromMe;
	}

	string toString () {
		if (myVal) {
			return "bool[true]";
		} else {
			return "bool[false]";
		}
	}	

	bool validate_tree(MyDB_CatalogPtr mycatalog)
	{
		return true;
	}

	string type_check(MyDB_CatalogPtr mycatalog)
	{
		return "bool";
	}

	bool in_group_clause(MyDB_CatalogPtr mycatalog){
		return true;
	}

	string get_tpye(){
		return "bool";
	}
};

class DoubleLiteral : public ExprTree {

private:
	double myVal;
public:

	DoubleLiteral (double fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "double[" + to_string (myVal) + "]";
	}	

	bool validate_tree(MyDB_CatalogPtr mycatalog)
	{
		return true;
	}

	string type_check(MyDB_CatalogPtr mycatalog)
	{
		return "double";
	}

	bool in_group_clause(MyDB_CatalogPtr mycatalog){
		return true;
	}

	string get_tpye(){
		return "double";
	}

	~DoubleLiteral () {}
};

// this implement class ExprTree
class IntLiteral : public ExprTree {

private:
	int myVal;
public:

	IntLiteral (int fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "int[" + to_string (myVal) + "]";
	}

	bool validate_tree(MyDB_CatalogPtr mycatalog)
	{
		return true;
	}

	string type_check(MyDB_CatalogPtr mycatalog)
	{
		return "int";
	}

	bool in_group_clause(MyDB_CatalogPtr mycatalog){
		return true;
	}

	string get_tpye(){
		return "int";
	}

	~IntLiteral () {}
};

class StringLiteral : public ExprTree {

private:
	string myVal;
public:

	StringLiteral (char *fromMe) {
		fromMe[strlen (fromMe) - 1] = 0;
		myVal = string (fromMe + 1);
	}

	string toString () {
		return "string[" + myVal + "]";
	}

	bool validate_tree(MyDB_CatalogPtr mycatalog)
	{
		return true;
	}

	string type_check(MyDB_CatalogPtr mycatalog)
	{
		return "string";
	}

	bool in_group_clause(MyDB_CatalogPtr mycatalog){
		return true;
	}

	string get_tpye(){
		return "string";
	}
	~StringLiteral () {}
};

class Identifier : public ExprTree {

private:
	string tableName;
	string attName;
public:

	Identifier (char *tableNameIn, char *attNameIn) {
		tableName = string (tableNameIn);
		attName = string (attNameIn);
	}

	string toString () {
		return "[" + tableName + "_" + attName + "]";
	}	

	bool validate_tree(MyDB_CatalogPtr mycatalog)
	{
		//cout<<"validating Identifier"<<endl;
		bool result = true;
		if(mycatalog->on_list(tableName) == -1 )
		{
			// see if table name is valid
			cout<<"table "<<tableName<<" doesn't exist"<<endl;
			result = false;
		}	
		else
		{
			// table name is valid, now check the attribute
			string type;
			string attr_name = tableName+"."+attName+".type";
			string alt_attr_name = mycatalog->get_full_name(tableName)+"."+attName+".type";
			if( (mycatalog->getString(alt_attr_name, type) == false)&& (mycatalog->getString(attr_name, type) == false) )
			{
				cout<<"attribute "<<attName<<" not found"<<endl;
				result = false;
			}
			// attribute found, its type is stored in sring type
		}
		return result;
	}

	string type_check(MyDB_CatalogPtr mycatalog)
	{
		string lhs_type, tableName1,attName1;
		tableName1=mycatalog->getTableName(toString());
		attName1= mycatalog->getAttName(toString());
		string attr_name1 = tableName1+"."+attName1+".type";	
		mycatalog->getString(attr_name1,lhs_type);		
		return lhs_type;
	}

	string get_tpye(){
		return "identifier";
	}

	bool in_group_clause(MyDB_CatalogPtr mycatalog){
		// TODO: implement the lookup group function
		if(mycatalog->on_grouping_list(tableName,attName)==-1)
		{
			cout<<tableName<<" and "<<attName<<" are not in grouping clause"<<endl;
			return false; // not on the grouping list
		}	
		else
			return true;	// on the grouping list
	}

	~Identifier () {}
};

class MinusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	MinusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "- (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool validate_tree(MyDB_CatalogPtr mycatalog)
	{
		//TODO: need to make sure two attribute can do math
		return ( lhs->validate_tree(mycatalog) && rhs->validate_tree(mycatalog));
	}

	string type_check(MyDB_CatalogPtr mycatalog)
	{
		string result="No";
		string lhs_type, rhs_type, tableName1,attName1, tableName2,attName2;
		if(lhs->get_tpye().compare("expression")==0 || rhs->get_tpye().compare("expression")==0)
		{
			lhs_type = lhs->type_check(mycatalog);
			rhs_type = rhs->type_check(mycatalog);
			if(lhs_type.compare("int")==0 || lhs_type.compare("double")==0)
			{
				if(rhs_type.compare("int")==0 || rhs_type.compare("double")==0)
				{
					if(lhs_type.compare("double")==0 || rhs_type.compare("double")==0)
						result = "double";
					else
						result = "int";
					return result;
				}
			}
			cout<<"Error: "<<lhs_type<<" - "<<rhs_type<<" type can not be matched!"<<endl;
			return result;
		}
		else if(lhs->get_tpye().compare("identifier")==0)
		{
			if(rhs->get_tpye().compare("identifier")==0)
			{
				tableName1=mycatalog->getTableName(lhs->toString());
				attName1= mycatalog->getAttName(lhs->toString());
				tableName2=mycatalog->getTableName(rhs->toString());
				attName2= mycatalog->getAttName(rhs->toString());
				string attr_name1 = tableName1+"."+attName1+".type";
				string attr_name2 = tableName2+"."+attName2+".type";
				mycatalog->getString(attr_name1,lhs_type);		
				mycatalog->getString(attr_name2,rhs_type);
			}
			else 
			{
				tableName1=mycatalog->getTableName(lhs->toString());
				attName1= mycatalog->getAttName(lhs->toString());
				string attr_name1 = tableName1+"."+attName1+".type";	
				mycatalog->getString(attr_name1,lhs_type);		
				rhs_type = rhs->get_tpye();
			}
	    }
		else 
		{
			if(rhs->get_tpye().compare("identifier")==0)
			{
				tableName2=mycatalog->getTableName(rhs->toString());
				attName2= mycatalog->getAttName(rhs->toString());
				string attr_name2 = tableName2+"."+attName2+".type";
				lhs_type = lhs->get_tpye();
				mycatalog->getString(attr_name2,rhs_type);
			}
			else 
			{
				lhs_type = lhs->get_tpye();	
				rhs_type = rhs->get_tpye();
			}
		}
		if(lhs_type.compare("int")==0 || lhs_type.compare("double")==0)
		{
			if(rhs_type.compare("int")==0 || rhs_type.compare("double")==0)
			{
			    if(lhs_type.compare("double")==0 || rhs_type.compare("double")==0)
					result = "double";
				else
					result = "int";
			}
		}
		if(result=="No")
		{
			cout<<"Error: "<<lhs_type<<" - "<<rhs_type<<" type can not be matched!"<<endl;
		}
		return result;
	}

	string get_tpye(){
		return "expression";
	}

	bool in_group_clause(MyDB_CatalogPtr mycatalog){
		return ( lhs->in_group_clause(mycatalog) && rhs->in_group_clause(mycatalog) );
	}

	~MinusOp () {}
};

class PlusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	PlusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "+ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool validate_tree(MyDB_CatalogPtr mycatalog)
	{
		//TODO: need to make sure two attribute can do math
		return ( lhs->validate_tree(mycatalog) && rhs->validate_tree(mycatalog));
	}

	string type_check(MyDB_CatalogPtr mycatalog)
	{
		string result="No";
		string lhs_type, rhs_type, tableName1,attName1, tableName2,attName2;
		if(lhs->get_tpye().compare("expression")==0 || rhs->get_tpye().compare("expression")==0)
		{
			lhs_type = lhs->type_check(mycatalog);
			rhs_type = rhs->type_check(mycatalog);
			if(lhs_type.compare("string")==0 && rhs_type.compare("string")==0)
			{
				result = "string";
				return result;
			}
			else if(lhs_type.compare("int")==0 || lhs_type.compare("double")==0)
			{
				if(rhs_type.compare("int")==0 || rhs_type.compare("double")==0)
				{
					if(lhs_type.compare("double")==0 || rhs_type.compare("double")==0)
						result = "double";
					else
						result = "int";
				    return result;
				}
			}
			if(result=="No")
			{
				cout<<"Error: "<<lhs_type<<" + "<<rhs_type<<" type can not be matched!"<<endl;
				return result;
			}
		}
		else if(lhs->get_tpye().compare("identifier")==0)
		{
			if(rhs->get_tpye().compare("identifier")==0)
			{
				tableName1=mycatalog->getTableName(lhs->toString());
				attName1= mycatalog->getAttName(lhs->toString());
				tableName2=mycatalog->getTableName(rhs->toString());
				attName2= mycatalog->getAttName(rhs->toString());
				string attr_name1 = tableName1+"."+attName1+".type";
				string attr_name2 = tableName2+"."+attName2+".type";
				mycatalog->getString(attr_name1,lhs_type);		
				mycatalog->getString(attr_name2,rhs_type);
			}
			else 
			{
				tableName1=mycatalog->getTableName(lhs->toString());
				attName1= mycatalog->getAttName(lhs->toString());
				string attr_name1 = tableName1+"."+attName1+".type";
				mycatalog->getString(attr_name1,lhs_type);		
				rhs_type = rhs->get_tpye();
			}
	    }
		else 
		{
			if(rhs->get_tpye().compare("identifier")==0)
			{
				tableName2=mycatalog->getTableName(rhs->toString());
				attName2= mycatalog->getAttName(rhs->toString());
				string attr_name2 = tableName2+"."+attName2+".type";
				lhs_type = lhs->get_tpye();
				mycatalog->getString(attr_name2,rhs_type);
			}
			else 
			{
				lhs_type = lhs->get_tpye();	
				rhs_type = rhs->get_tpye();
			}
		}
		if(lhs_type.compare("string")==0 && rhs_type.compare("string")==0)
		{
			result = "string";
		}
		else if(lhs_type.compare("int")==0 || lhs_type.compare("double")==0)
		{
			if(rhs_type.compare("int")==0 || rhs_type.compare("double")==0)
			{
			    if(lhs_type.compare("double")==0 || rhs_type.compare("double")==0)
					result = "double";
				else
					result = "int";
			}
		}
		if(result=="No")
		{
			cout<<"Error: "<<lhs_type<<" + "<<rhs_type<<" type can not be matched!"<<endl;
		}
		return result;
	}

	string get_tpye(){
		return "expression";
	}


	bool in_group_clause(MyDB_CatalogPtr mycatalog){
		return ( lhs->in_group_clause(mycatalog) && rhs->in_group_clause(mycatalog) );
	}

	~PlusOp () {}
};

class TimesOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	TimesOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "* (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool validate_tree(MyDB_CatalogPtr mycatalog)
	{
		//TODO: need to make sure two attribute can do math
		return ( lhs->validate_tree(mycatalog) && rhs->validate_tree(mycatalog));
	}

	string get_tpye(){
		return "expression";
	}

	string type_check(MyDB_CatalogPtr mycatalog)
	{
		string result="No";
		string lhs_type, rhs_type, tableName1,attName1, tableName2,attName2;
		if(lhs->get_tpye().compare("expression")==0 || rhs->get_tpye().compare("expression")==0)
		{
			lhs_type = lhs->type_check(mycatalog);
			rhs_type = rhs->type_check(mycatalog);
			if(lhs_type.compare("int")==0 || lhs_type.compare("double")==0)
			{
				if(rhs_type.compare("int")==0 || rhs_type.compare("double")==0)
				{
					if(lhs_type.compare("double")==0 || rhs_type.compare("double")==0)
						result = "double";
					else
						result = "int";
					return result;
				}
			}
			if(result=="No")
			{
				cout<<"Error: "<<lhs_type<<" * "<<rhs_type<<" type can not be matched!"<<endl;
				return result;
			}

		}
		else if(lhs->get_tpye().compare("identifier")==0)
		{
			if(rhs->get_tpye().compare("identifier")==0)
			{
				tableName1=mycatalog->getTableName(lhs->toString());
				attName1= mycatalog->getAttName(lhs->toString());
				tableName2=mycatalog->getTableName(rhs->toString());
				attName2= mycatalog->getAttName(rhs->toString());
				string attr_name1 = tableName1+"."+attName1+".type";
				string attr_name2 = tableName2+"."+attName2+".type";
				mycatalog->getString(attr_name1,lhs_type);		
				mycatalog->getString(attr_name2,rhs_type);
			}
			else 
			{
				tableName1=mycatalog->getTableName(lhs->toString());
				attName1= mycatalog->getAttName(lhs->toString());
				string attr_name1 = tableName1+"."+attName1+".type";
				mycatalog->getString(attr_name1,lhs_type);		
				rhs_type = rhs->get_tpye();
			}
	    }
		else 
		{
			if(rhs->get_tpye().compare("identifier")==0)
			{
				tableName2=mycatalog->getTableName(rhs->toString());
				attName2= mycatalog->getAttName(rhs->toString());
				string attr_name2 = tableName2+"."+attName2+".type";
				lhs_type = lhs->get_tpye();
				mycatalog->getString(attr_name2,rhs_type);
			}
			else 
			{
				lhs_type = lhs->get_tpye();	
				rhs_type = rhs->get_tpye();
			}
		}
		if(lhs_type.compare("int")==0 || lhs_type.compare("double")==0)
		{
			if(rhs_type.compare("int")==0 || rhs_type.compare("double")==0)
			{
				if(lhs_type.compare("double")==0 || rhs_type.compare("double")==0)
					result = "double";
				else
					result = "int";
			}
		}
		if(result=="No")
		{
			cout<<"Error: "<<lhs_type<<" * "<<rhs_type<<" type can not be matched!"<<endl;
		}
		return result;
	}

	bool in_group_clause(MyDB_CatalogPtr mycatalog){
		return ( lhs->in_group_clause(mycatalog) && rhs->in_group_clause(mycatalog) );
	}

	~TimesOp () {}
};

class DivideOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	DivideOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "/ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool validate_tree(MyDB_CatalogPtr mycatalog)
	{
		//TODO: need to make sure two attribute can do math
		return ( lhs->validate_tree(mycatalog) && rhs->validate_tree(mycatalog));
	}

	string get_tpye(){
		return "expression";
	}

	string type_check(MyDB_CatalogPtr mycatalog)
	{
		string result="No";
		string lhs_type, rhs_type, tableName1,attName1, tableName2,attName2;
		if(lhs->get_tpye().compare("expression")==0 || rhs->get_tpye().compare("expression")==0)
		{
			lhs_type = lhs->type_check(mycatalog);
			rhs_type = rhs->type_check(mycatalog);
			if(lhs_type.compare("int")==0 || lhs_type.compare("double")==0)
			{
				if(rhs_type.compare("int")==0 || rhs_type.compare("double")==0)
				{
					if(lhs_type.compare("double")==0 || rhs_type.compare("double")==0)
						result = "double";
					else
						result = "int";
					return result;
				}
			}
			if(result=="No")
			{
				cout<<"Error: "<<lhs_type<<" / "<<rhs_type<<" type can not be matched!"<<endl;
				return result;
			}
		}
		else if(lhs->get_tpye().compare("identifier")==0)
		{
			if(rhs->get_tpye().compare("identifier")==0)
			{
				tableName1=mycatalog->getTableName(lhs->toString());
				attName1= mycatalog->getAttName(lhs->toString());
				tableName2=mycatalog->getTableName(rhs->toString());
				attName2= mycatalog->getAttName(rhs->toString());
				string attr_name1 = tableName1+"."+attName1+".type";
				string attr_name2 = tableName2+"."+attName2+".type";
				mycatalog->getString(attr_name1,lhs_type);		
				mycatalog->getString(attr_name2,rhs_type);
			}
			else 
			{
				tableName1=mycatalog->getTableName(lhs->toString());
				attName1= mycatalog->getAttName(lhs->toString());
				string attr_name1 = tableName1+"."+attName1+".type";
				mycatalog->getString(attr_name1,lhs_type);		
				rhs_type = rhs->get_tpye();
			}
	    }
		else 
		{
			if(rhs->get_tpye().compare("identifier")==0)
			{
				tableName2=mycatalog->getTableName(rhs->toString());
				attName2= mycatalog->getAttName(rhs->toString());
				string attr_name2 = tableName2+"."+attName2+".type";
				lhs_type = lhs->get_tpye();
				mycatalog->getString(attr_name2,rhs_type);
			}
			else 
			{
				lhs_type = lhs->get_tpye();	
				rhs_type = rhs->get_tpye();
			}
		}
		if(lhs_type.compare("int")==0 || lhs_type.compare("double")==0)
		{
			if(rhs_type.compare("int")==0 || rhs_type.compare("double")==0)
			{
			    if(lhs_type.compare("double")==0 || rhs_type.compare("double")==0)
					result = "double";
				else
					result = "int";
			}
		}
		if(result=="No")
		{
			cout<<"Error: "<<lhs_type<<" / "<<rhs_type<<" type can not be matched!"<<endl;
		}
		return result;
	}

	bool in_group_clause(MyDB_CatalogPtr mycatalog){
		return ( lhs->in_group_clause(mycatalog) && rhs->in_group_clause(mycatalog) );
	}

	~DivideOp () {}
};

class GtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	GtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "> (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool validate_tree(MyDB_CatalogPtr mycatalog)
	{
		//TODO: need to make sure two attribute can do math
		return ( lhs->validate_tree(mycatalog) && rhs->validate_tree(mycatalog));
	}

	string get_tpye(){
		return "bool";
	}

	string type_check(MyDB_CatalogPtr mycatalog)
	{
		string result="No";
		string lhs_type, rhs_type, tableName1,attName1, tableName2,attName2;
		if(lhs->get_tpye().compare("expression")==0 || rhs->get_tpye().compare("expression")==0)
		{
			lhs_type = lhs->type_check(mycatalog);
			rhs_type = rhs->type_check(mycatalog);
			if(lhs_type.compare("string")==0 && rhs_type.compare("string")==0)
			{
				result = "bool";
				return result;
			}
			else if(lhs_type.compare("int")==0 || lhs_type.compare("double")==0)
			{
				if(rhs_type.compare("int")==0 || rhs_type.compare("double")==0)
				{
					result = "bool";
					return result;
				}
			}
			if(result=="No")
			{
				cout<<"Error: "<<lhs_type<<" > "<<rhs_type<<" type can not be matched!"<<endl;
				return result;
			}
		}
		else if(lhs->get_tpye().compare("identifier")==0)
		{
			if(rhs->get_tpye().compare("identifier")==0)
			{
				tableName1=mycatalog->getTableName(lhs->toString());
				attName1= mycatalog->getAttName(lhs->toString());
				tableName2=mycatalog->getTableName(rhs->toString());
				attName2= mycatalog->getAttName(rhs->toString());
				string attr_name1 = tableName1+"."+attName1+".type";
				string attr_name2 = tableName2+"."+attName2+".type";	
				mycatalog->getString(attr_name1,lhs_type);		
				mycatalog->getString(attr_name2,rhs_type);
			}
			else 
			{
				tableName1=mycatalog->getTableName(lhs->toString());
				attName1= mycatalog->getAttName(lhs->toString());
				string attr_name1 = tableName1+"."+attName1+".type";
				mycatalog->getString(attr_name1,lhs_type);		
				rhs_type = rhs->get_tpye();
			}
	    }
		else 
		{
			if(rhs->get_tpye().compare("identifier")==0)
			{
				tableName2=mycatalog->getTableName(rhs->toString());
				attName2= mycatalog->getAttName(rhs->toString());
				string attr_name2 = tableName2+"."+attName2+".type";
				lhs_type = lhs->get_tpye();
				mycatalog->getString(attr_name2,rhs_type);
			}
			else 
			{
				lhs_type = lhs->get_tpye();	
				rhs_type = rhs->get_tpye();
			}
		}


		if(lhs_type.compare("string")==0 && rhs_type.compare("string")==0)
		{
			result = "bool";
		}
		else if(lhs_type.compare("int")==0 || lhs_type.compare("double")==0)
		{
			if(rhs_type.compare("int")==0 || rhs_type.compare("double")==0)
			{
			    result = "bool";
			}
		}
		if(result=="No")
		{
			cout<<"Error: "<<lhs_type<<" > "<<rhs_type<<" type can not be matched!"<<endl;
		}
		return result;
	}

	bool in_group_clause(MyDB_CatalogPtr mycatalog){
		return ( lhs->in_group_clause(mycatalog) && rhs->in_group_clause(mycatalog) );
	}

	~GtOp () {}
};

class LtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	LtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "< (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool validate_tree(MyDB_CatalogPtr mycatalog)
	{
		//TODO: need to make sure two attribute can do math
		return ( lhs->validate_tree(mycatalog) && rhs->validate_tree(mycatalog));
	}

	string get_tpye(){
		return "bool";
	}

	string type_check(MyDB_CatalogPtr mycatalog)
	{
		string result="No";
		string lhs_type, rhs_type, tableName1,attName1, tableName2,attName2;
		if(lhs->get_tpye().compare("expression")==0 || rhs->get_tpye().compare("expression")==0)
		{
			lhs_type = lhs->type_check(mycatalog);
			rhs_type = rhs->type_check(mycatalog);
			if(lhs_type.compare("string")==0 && rhs_type.compare("string")==0)
			{
				result = "bool";
				return result;
			}
			else if(lhs_type.compare("int")==0 || lhs_type.compare("double")==0)
			{
				if(rhs_type.compare("int")==0 || rhs_type.compare("double")==0)
				{
					result = "bool";
					return result;
				}
			}
			if(result=="No")
			{
				cout<<"Error: "<<lhs_type<<" < "<<rhs_type<<" type can not be matched!"<<endl;
				return result;
			}
		}
		else if(lhs->get_tpye().compare("identifier")==0)
		{
			if(rhs->get_tpye().compare("identifier")==0)
			{
				tableName1=mycatalog->getTableName(lhs->toString());
				attName1= mycatalog->getAttName(lhs->toString());
				tableName2=mycatalog->getTableName(rhs->toString());
				attName2= mycatalog->getAttName(rhs->toString());
				string attr_name1 = tableName1+"."+attName1+".type";
				string attr_name2 = tableName2+"."+attName2+".type";
				mycatalog->getString(attr_name1,lhs_type);		
				mycatalog->getString(attr_name2,rhs_type);
			}
			else 
			{
				tableName1=mycatalog->getTableName(lhs->toString());
				attName1= mycatalog->getAttName(lhs->toString());
				string attr_name1 = tableName1+"."+attName1+".type";
				mycatalog->getString(attr_name1,lhs_type);		
				rhs_type = rhs->get_tpye();
			}
	    }
		else 
		{
			if(rhs->get_tpye().compare("identifier")==0)
			{
				tableName2=mycatalog->getTableName(rhs->toString());
				attName2= mycatalog->getAttName(rhs->toString());
				string attr_name2 = tableName2+"."+attName2+".type";
				lhs_type = lhs->get_tpye();
				mycatalog->getString(attr_name2,rhs_type);
			}
			else 
			{
				lhs_type = lhs->get_tpye();	
				rhs_type = rhs->get_tpye();
			}
		}
		if(lhs_type.compare("string")==0 && rhs_type.compare("string")==0)
		{
			result = "bool";
		}
		else if(lhs_type.compare("int")==0 || lhs_type.compare("double")==0)
		{
			if(rhs_type.compare("int")==0 || rhs_type.compare("double")==0)
			{
			    result = "bool";
			}
		}
		if(result=="No")
		{
			cout<<"Error: "<<lhs_type<<" < "<<rhs_type<<" type can not be matched!"<<endl;
		}
		return result;
	}

	bool in_group_clause(MyDB_CatalogPtr mycatalog){
		return ( lhs->in_group_clause(mycatalog) && rhs->in_group_clause(mycatalog) );
	}

	~LtOp () {}
};

class NeqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	NeqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "!= (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool validate_tree(MyDB_CatalogPtr mycatalog)
	{
		//TODO: need to make sure two attribute can do math
		return ( lhs->validate_tree(mycatalog) && rhs->validate_tree(mycatalog));
	}

	string get_tpye(){
		return "bool";
	}

	string type_check(MyDB_CatalogPtr mycatalog)
	{
		string result = "No";
		string lhs_type, rhs_type, tableName1,attName1, tableName2,attName2;
		if(lhs->get_tpye().compare("expression")==0 || rhs->get_tpye().compare("expression")==0)
		{
			lhs_type = lhs->type_check(mycatalog);
			rhs_type = rhs->type_check(mycatalog);
			if(lhs_type.compare("string")==0 && rhs_type.compare("string")==0)
			{
				result = "bool";
				return result;
			}
			else if(lhs_type.compare("int")==0 || lhs_type.compare("double")==0)
			{
				if(rhs_type.compare("int")==0 || rhs_type.compare("double")==0)
				{
					result = "bool";
					return result;
				}
			}
			if(result=="No")
			{
				cout<<"Error: "<<lhs_type<<" != "<<rhs_type<<" type can not be matched!"<<endl;
				return result;
			}
		}
		else if(lhs->get_tpye().compare("identifier")==0)
		{
			if(rhs->get_tpye().compare("identifier")==0)
			{
				tableName1=mycatalog->getTableName(lhs->toString());
				attName1= mycatalog->getAttName(lhs->toString());
				tableName2=mycatalog->getTableName(rhs->toString());
				attName2= mycatalog->getAttName(rhs->toString());
				string attr_name1 = tableName1+"."+attName1+".type";
				string attr_name2 = tableName2+"."+attName2+".type";
				mycatalog->getString(attr_name1,lhs_type);		
				mycatalog->getString(attr_name2,rhs_type);
			}
			else 
			{
				tableName1=mycatalog->getTableName(lhs->toString());
				attName1= mycatalog->getAttName(lhs->toString());
				string attr_name1 = tableName1+"."+attName1+".type";
				mycatalog->getString(attr_name1,lhs_type);		
				rhs_type = rhs->get_tpye();
			}
	    }
		else 
		{
			if(rhs->get_tpye().compare("identifier")==0)
			{
				tableName2=mycatalog->getTableName(rhs->toString());
				attName2= mycatalog->getAttName(rhs->toString());
				string attr_name2 = tableName2+"."+attName2+".type";
				lhs_type = lhs->get_tpye();
				mycatalog->getString(attr_name2,rhs_type);
			}
			else 
			{
				lhs_type = lhs->get_tpye();	
				rhs_type = rhs->get_tpye();
			}
		}
		if(lhs_type.compare("string")==0 && rhs_type.compare("string")==0)
		{
			result = "bool";
		}
		else if(lhs_type.compare("int")==0 || lhs_type.compare("double")==0)
		{
			if(rhs_type.compare("int")==0 || rhs_type.compare("double")==0)
			{
			    result = "bool";
			}
		}
		if(result=="No")
		{
			cout<<"Error: "<<lhs_type<<" != "<<rhs_type<<" type can not be matched!"<<endl;
		}
		return result;
	}

	bool in_group_clause(MyDB_CatalogPtr mycatalog){
		return ( lhs->in_group_clause(mycatalog) && rhs->in_group_clause(mycatalog) );
	}

	~NeqOp () {}
};

class OrOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	OrOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "|| (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool validate_tree(MyDB_CatalogPtr mycatalog)
	{
		//TODO: need to make sure two attribute can do math
		return ( lhs->validate_tree(mycatalog) && rhs->validate_tree(mycatalog));
	}

	string get_tpye(){
		return "expression";
	}

	string type_check(MyDB_CatalogPtr mycatalog)
	{
		string result = "No";
		string lhs_type, rhs_type, tableName1,attName1, tableName2,attName2;
		if(lhs->get_tpye().compare("bool")==0 && rhs->get_tpye().compare("bool")==0)
		{
			if(lhs->type_check(mycatalog)=="bool" && rhs->type_check(mycatalog)=="bool")
			{
				result="bool";
				return result;
			}
			if(result=="No")
			{
				cout<<"Error: "<<lhs_type<<" || "<<rhs_type<<" type can not be matched!"<<endl;
				return result;
			}
		}
		else
		{
			if(result=="No")
			{
				cout<<"Error: "<<lhs_type<<" || "<<rhs_type<<" type can not be matched!"<<endl;
			}
			return result;
		}
		return "No";
	}

	bool in_group_clause(MyDB_CatalogPtr mycatalog){
		return ( lhs->in_group_clause(mycatalog) && rhs->in_group_clause(mycatalog) );
	}

	~OrOp () {}
};

class EqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	EqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "== (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool validate_tree(MyDB_CatalogPtr mycatalog)
	{
		//TODO: need to make sure two attribute can do math
		return ( lhs->validate_tree(mycatalog) && rhs->validate_tree(mycatalog));
	}

	string get_tpye(){
		return "bool";
	}

	string type_check(MyDB_CatalogPtr mycatalog)
	{
		string result = "No";
		string lhs_type, rhs_type, tableName1,attName1, tableName2,attName2;
		if(lhs->get_tpye().compare("expression")==0 || rhs->get_tpye().compare("expression")==0)
		{
			lhs_type = lhs->type_check(mycatalog);
			rhs_type = rhs->type_check(mycatalog);
			if(lhs_type.compare("string")==0 && rhs_type.compare("string")==0)
			{
				result = "bool";
				return result;
			}
			else if(lhs_type.compare("int")==0 || lhs_type.compare("double")==0)
			{
				if(rhs_type.compare("int")==0 || rhs_type.compare("double")==0)
				{
					result = "bool";
					return result;
				}
			}
			if(result=="No")
			{
				cout<<"Error: "<<lhs_type<<" = "<<rhs_type<<" type can not be matched!"<<endl;
				return result;
			}
		}
		else if(lhs->get_tpye().compare("identifier")==0)
		{
			if(rhs->get_tpye().compare("identifier")==0)
			{
				tableName1=mycatalog->getTableName(lhs->toString());
				attName1= mycatalog->getAttName(lhs->toString());
				tableName2=mycatalog->getTableName(rhs->toString());
				attName2= mycatalog->getAttName(rhs->toString());
				string attr_name1 = tableName1+"."+attName1+".type";
				string attr_name2 = tableName2+"."+attName2+".type";
				mycatalog->getString(attr_name1,lhs_type);		
				mycatalog->getString(attr_name2,rhs_type);
			}
			else 
			{
				tableName1=mycatalog->getTableName(lhs->toString());
				attName1= mycatalog->getAttName(lhs->toString());
				string attr_name1 = tableName1+"."+attName1+".type";
				mycatalog->getString(attr_name1,lhs_type);		
				rhs_type = rhs->get_tpye();
			}
	    }
		else 
		{
			if(rhs->get_tpye().compare("identifier")==0)
			{
				tableName2=mycatalog->getTableName(rhs->toString());
				attName2= mycatalog->getAttName(rhs->toString());
				string attr_name2 = tableName2+"."+attName2+".type";
				lhs_type = lhs->get_tpye();
				mycatalog->getString(attr_name2,rhs_type);
			}
			else 
			{
				lhs_type = lhs->get_tpye();	
				rhs_type = rhs->get_tpye();
			}
		}

		if(lhs_type.compare("string")==0 && rhs_type.compare("string")==0)
		{
			result = "bool";
		}
		else if(lhs_type.compare("int")==0 || lhs_type.compare("double")==0)
		{
			if(rhs_type.compare("int")==0 || rhs_type.compare("double")==0)
			{
			    result = "bool";
			}
		}
		if(result=="No")
		{
			cout<<"Error: "<<lhs_type<<" = "<<rhs_type<<" type can not be matched!"<<endl;
		}
		return result;
	}

	bool in_group_clause(MyDB_CatalogPtr mycatalog){
		return ( lhs->in_group_clause(mycatalog) && rhs->in_group_clause(mycatalog) );
	}

	~EqOp () {}
};

class NotOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	NotOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "!(" + child->toString () + ")";
	}	

	bool validate_tree(MyDB_CatalogPtr mycatalog)
	{
		//TODO: need to make sure child is valid
		return child->validate_tree(mycatalog);
	}

	string get_tpye(){
		return "bool";
	}

	string type_check(MyDB_CatalogPtr mycatalog)
	{
		string result = "No";
		string child_type;
		if(child->get_tpye().compare("bool")==0 )
		{
//			cout<<child->toString ()<<endl;
			if(child->type_check(mycatalog)=="bool" )
			{
				result="bool";
				return result;
			}
			if(result=="No")
			{
				cout<<"Error: "<<" ! "<<child->get_tpye()<<" type can not be matched!"<<endl;
				return result;
			}
		}
		else
		{
			if(result=="No")
			{
				cout<<"Error: "<<" ! "<<child->get_tpye()<<" type can not be matched!"<<endl;
			}
			return result;
		}
		return "No";
	}

	bool in_group_clause(MyDB_CatalogPtr mycatalog){
		//return child->in_group_clause(mycatalog);
		return true;
	}

	~NotOp () {}
};

class SumOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	SumOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "sum(" + child->toString () + ")";
	}	

	bool validate_tree(MyDB_CatalogPtr mycatalog)
	{
		//TODO: need to make sure child can be sum up
		return child->validate_tree(mycatalog);
	}

	string get_tpye(){
		return "expression";
	}

	string type_check(MyDB_CatalogPtr mycatalog)
	{
		string result = "No";
		string child_type;
		if(child->get_tpye().compare("expression")==0 )
		{
			child_type = child->type_check(mycatalog);
			if(child_type=="int" || child_type=="double"  )
			{
				result="int";
				return result;
			}
			if(result=="No")
			{
				cout<<"Error: "<<" sum("<<child_type<<") type can not be matched!"<<endl;
				return result;
			}
		}
		else
		{
			if(result=="No")
			{
				cout<<"Error: "<<" sum("<<child_type<<") type can not be matched!"<<endl;
			}
			return result;
		}
		return "No";
	}



	bool in_group_clause(MyDB_CatalogPtr mycatalog){
		//return child->in_group_clause(mycatalog);
		return true;
	}

	~SumOp () {}
};

class AvgOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	AvgOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "avg(" + child->toString () + ")";
	}	

	bool validate_tree(MyDB_CatalogPtr mycatalog)
	{
		//TODO: need to make sure child can to average
		return child->validate_tree(mycatalog);
	}

	string get_tpye(){
		return "expression";
	}

	string type_check(MyDB_CatalogPtr mycatalog)
	{
		string result = "No";
		string child_type;
		if(child->get_tpye().compare("expression")==0 )
		{
			child_type = child->type_check(mycatalog);
			if(child_type=="int" || child_type=="double"  )
			{
				result="double";
				return result;
			}
			if(result=="No")
			{
				cout<<"Error: "<<" avg("<<child_type<<") type can not be matched!"<<endl;
				return result;
			}
		}
		else
		{
			if(result=="No")
			{
				cout<<"Error: "<<" avg("<<child_type<<") type can not be matched!"<<endl;
			}
			return result;
		}
		return "No";
	}


	bool in_group_clause(MyDB_CatalogPtr mycatalog){
		//return child->in_group_clause(mycatalog);
		return true;
	}

	~AvgOp () {}
};


#endif
