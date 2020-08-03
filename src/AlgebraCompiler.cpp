// AlgebraCompiler.cpp
//
// Copyright (c) 1998--2020 Krishna Myneni
//
// This software is provided under the terms of the
// GNU Affero General Public License (AGPL) v 3.0 or later.
//
#include <iostream>
#include <vector>
#include <stack>
#include <deque>
using namespace std;
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "fbc.h"
#define byte unsigned char

#define PREFIX_CODE_SIZE 18
const char* STR_ADD = "+";
const char* STR_SUB = "-";
const char* STR_NEG = "N";
const char* STR_MUL = "*";
const char* STR_DIV = "/";
const char* STR_POW = "^";
const char* STR_UNK = "?";

int CompileAE ( vector<byte>*, char* );
const char* OpcodeToStr( byte );
byte StrToOpcode( char*, char* );
bool IsOperator( char* );
bool IsUnaryOp( byte );
void PushNumeric( double, deque<byte>* );
int PushOperators ( stack<byte>*, deque<byte>*, int );
int HoldOperator( byte, stack<byte>*, deque<byte>*, int );

// Compile algebraic expression into a vector of kForth Forth Byte Codes.
// Compiler assumes that upon execution of these tokens, the Forth
// stack contains the following:
//
//      addr  nsize  npts
//
// where addr is the starting address of the data, nsize is the
// number of elements (floats) in a datum, and npts is the number
// of points.
//
// The stack diagram for an expression evaluation is:
//
//      addr  nsize  npts  --
//
// Return value for CompileAE:
//
//  0   no error
//  1   syntax error due to unrecognized character
//  2   syntax error due to unrecognized name in expression
//  3   destination array (X or Y) cannot be determined     
//  4   syntax error due to unmatched operators for operands

int CompileAE (vector<byte>* pOpCodes, char* exp)
{
    const char* wspace = " \t\n\r";

    stack< byte > hs;    // operator hold stack
    deque< byte > op;    // sequence of operators in double ended queue

    char exp_copy [256];
    char LabelName [256];
    int i, ival, operands_pending = 0, ecode = 0;
    char *tp, *cp, *endp;
    byte final_op = 0, temp, ch, *bp;
    double f;

    // Copy the expression removing all white space
    cp = exp;
    tp = exp_copy;
    while (*cp) {
      if (!strchr(wspace, *cp))
	*tp++ = *cp++;
      else
	++cp;
    }
    *tp = (char) 0;
    tp = exp_copy;

    // Set up the loop parameters
    op.push_back(OP_SWAP);      // bring nsize to top
    op.push_back(OP_IVAL);
    ival = sizeof(float);
    bp = (byte*)&ival;
    for (i = 0; i < sizeof(int); i++)
        op.push_back(*(bp + i));
    op.push_back(OP_MUL);       // multiply nsize by sizeof(float)
    op.push_back(OP_SWAP);      // bring npts to top
    op.push_back(OP_IVAL);
    for (i = 0; i < sizeof(int); i++) op.push_back(0);
    op.push_back(OP_PUSH);      // loop starting index
    op.push_back(OP_PUSH);      // loop count
    op.push_back(OP_PUSHIP);    // beginning of loop
    op.push_back(OP_PUSH);      // push nbytes onto return stack
    op.push_back(OP_PUSH);      // push address onto return stack

    cp = tp;    // set character ptr to first char in token
    while (*cp) {
      if (IsOperator(cp)) {
	ch = StrToOpcode(cp, tp);
        operands_pending = HoldOperator( ch, &hs, &op, operands_pending ); 
	++cp;
      }
      else if ((f = strtod(cp, &endp)) != 0.) {
	// push a numeric constant operand into the opcode queue
	PushNumeric(f, &op);
         ++operands_pending;  
         cp = endp;          // advance character ptr
      }
      else if (isalpha(*cp)) {
        if (*cp == 'X') {    // fetch x
          // cout << "Push X" << endl;
          op.push_back(OP_RFETCH);
          op.push_back(OP_SFFETCH);
          if (! final_op) final_op = 1;
          ++operands_pending;
        }
        else if (*cp == 'Y') {  // fetch
	  // cout << "Push Y" << endl;
          op.push_back(OP_RFETCH);
	  op.push_back(OP_CELLPLUS);
          op.push_back(OP_SFFETCH);
          if (! final_op) final_op = 2;
          ++operands_pending;
        }
        // else if (IsOperator(LabelName, &temp))
        // {
        // push function operator onto the hold stack
        // hs.push(temp);
        // }
        else {
          ecode = 2;  // error: unrecognized name
          break;
        }
        ++cp;
      }
      else if (*cp == '=') {
        while (op.size() > PREFIX_CODE_SIZE) {
          byte c = op.back();
          op.pop_back();
          // cout << "Pop " << *OpcodeToStr(c) << endl;
        }
        operands_pending = 0;
	while (!hs.empty()) hs.pop();
        ++cp;
	tp = cp;
      }
      else {                   
        temp = *cp;         // for debug use
        ecode = 1;  // error: syntax error in expression
        break;
      }
   }  // end while

// endloop:

    if (! final_op) ecode = 3;   // error: output destination undetermined

    if (hs.size()) {
      // Push remaining operators on hold stack
      operands_pending = PushOperators(&hs, &op, operands_pending);
      if (hs.size()) ecode = 4;  // error: unmatched operators
    }

    if (operands_pending != 1) ecode = 4; // error: unmatched operands

    if (ecode == 0) {
      op.push_back(OP_POP);
      op.push_back(OP_DUP);
      op.push_back(OP_2SWAP);
      op.push_back(OP_ROT);

      if (final_op == 1) {
            ;
      }
      else if (final_op == 2) {
	op.push_back(OP_CELLPLUS);
      }
      else
        ;
      op.push_back(OP_SFSTORE);
      op.push_back(OP_RFETCH);
      op.push_back(OP_ADD);   // advance data ptr by nbytes
      op.push_back(OP_POP);
      op.push_back(OP_RTLOOP);
      op.push_back(OP_2DROP); // clean up the stack
      op.push_back(OP_RET);
    }

    // Copy the opcode sequences into the destination vector
    for (i = 0; i < op.size(); i++) pOpCodes->push_back(op[i]);
    return ecode;
}

void PushNumeric(double f, deque<byte>* pOp)
{
    pOp->push_back(OP_FVAL);
    byte* bp = (byte*) &f;
    for (int i = 0; i < sizeof(double); i++)
      pOp->push_back(*(bp+i));
    // cout << "Push " << f << endl;
    return;
}

bool IsOperator( char* pStr )
{
    const char* arm_op = "^*/+-";
    return( strchr(arm_op, *pStr) );
}

bool IsUnaryOp (byte opcode)
{
    // Return true if op code is for a unary operator
    bool flag = false;
    switch (opcode) {
      case OP_FNEG:
      case OP_FABS:
      case OP_FCOS:
      case OP_FSIN:
      case OP_FTAN:
      case OP_FEXP:
      case OP_FLOG:
      case OP_FLN:
      case OP_FACOS:
      case OP_FASIN:
      case OP_FROUND:
      case OP_FTRUNC:
        flag = true;
	break;
      default:
	break;
    }
    return flag;
}


byte StrToOpcode( char* pOpStr, char* pOpStrStart )
{
    char* cp = pOpStr;
    char* tp = pOpStrStart;
    char c = *cp;
    byte ch;
    if (c == '^')
      ch = OP_FPOW;
    else if (c == '*')
      ch = OP_FMUL;
    else if (c == '/')
      ch = OP_FDIV;
    else if (c == '+')
      // is '+' a sign marker or addition operator?
      ch = ((cp == tp) || IsOperator(cp-1)) ? 0 : OP_FADD;
    else if (c == '-') { 
      // is '-' a monadic or dyadic operator?
      ch = ((cp == tp) || IsOperator(cp-1)) ? OP_FNEG : OP_FSUB;
    }
    else
      ch = 0;  // unhandled operator; skip char
    
    return( ch );
}

const char* OpcodeToStr(byte op)
{
    const char* p;
    switch (op) {
      case OP_FADD:
        p = STR_ADD;
        break;
      case OP_FSUB:
        p = STR_SUB;
        break;
      case OP_FMUL:
        p = STR_MUL;
        break;
      case OP_FDIV:
        p = STR_DIV;
        break;
      case OP_FPOW:
        p = STR_POW;
        break;
      case OP_FNEG:
        p = STR_NEG;
        break;
      default:
        p = STR_UNK;
        break;
    }
    return p;
}

int HoldOperator(byte ch, stack<byte>* pH, deque<byte>* pOp, int pending)
{
    if (ch) { 
      if (!IsUnaryOp(ch)) {
	if (!pH->empty()) {
	  byte last_op = pH->top();
	  byte next_to_last = 0;
	  if (pH->size() > 1) {
	    pH->pop();
	    next_to_last = pH->top(); // we need next to last
	    pH->push(last_op);
	  }
          if ( 
            (ch == OP_FPOW) ||
            (((ch == OP_FMUL) || (ch == OP_FDIV)) &&
            ((last_op == OP_FADD) || (last_op == OP_FSUB) )) ||
            ((!(last_op == OP_FNEG)) && (next_to_last == OP_FPOW))
          ) {
	      // current operator has higher precedence than last
              // cout << "C>L: ";
	      ;
          }
          else {
	    // last operator has higher or equal precedence than current
	    // cout << "L>C: ";
	    pending = PushOperators(pH, pOp, pending);
	  }
	}
      }
      pH->push(ch);
      // cout << "Hold " << *OpcodeToStr(ch) << endl;
      // cout << "Held items = " << pH->size() << endl;
    }
    return pending;
}

int PushOperators ( stack<byte>* pH, deque<byte>* pOp, int pending )
{
    // Push operators from hold stack into the opcode queue,
    // if there are sufficient operands available.
    byte last_op;
    while ( (!pH->empty()) && pending) {
      last_op = pH->top();		
      if (IsUnaryOp(last_op)) {
        pOp->push_back(last_op);
      }
      else if (pending > 1) {
        pOp->push_back(last_op);
        --pending;
      }
      else
       break;

      // cout << "Push " << *OpcodeToStr(last_op) << endl;
      pH->pop();
    }
    return pending;
}

