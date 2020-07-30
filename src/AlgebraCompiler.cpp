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

int CompileAE (vector<byte>*, char* exp);
char OpChar(byte);
bool IsMonadic(byte);
int PushOperators ( stack<byte>*, deque<byte>*, int);

int CompileAE (vector<byte>* pOpCodes, char* exp)
{
// Compile algebraic expression into a vector of CCRE Forth Byte Codes.
// Compiler assumes that upon execution of these tokens, the Forth
//   stack contains the following:
//
//      addr  nsize  npts
//
// where addr is the starting address of the data, nsize is the
// number of elements (floats) in a datum, and npts is the number
// of points.
// Therefore, the stack diagram for an expression evaluation is:
//
//      addr  nsize  npts  --
//
// Return value for CompileAE:
//
//  0   no error.
//

    const char* wspace = " \t\n\r";
    const char* arm_op = "^*/+-";

    stack< byte > hs;    // operator hold stack
    deque< byte > op;    // sequence of operators in double ended queue

    char exp_copy [256];
    char LabelName [256];
    int i, ival, operands_pending = 0, ecode = 0;
    char *tp, *cp, *endp;
    byte last_op = 0, final_op = 0, temp, ch, *bp;
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

    // set up the loop parameters

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
      if (strchr (arm_op, *cp)) {
        // It's an arithmetic operator
        ch = *cp;
        if (ch == '^')
          ch = OP_FPOW;
        else if (ch == '*')
          ch = OP_FMUL;
        else if (ch == '/')
          ch = OP_FDIV;
        else if (ch == '+')
	  // is '+' a sign marker or a dyadic operator?
          ch = ((cp == tp) || (strchr(arm_op, *(cp-1)))) ? 0 : OP_FADD;
        else if (ch == '-') { 
	  // is '-' a monadic or dyadic operator?
	  ch = ((cp == tp) || (strchr(arm_op, *(cp-1)))) ? OP_FNEG : OP_FSUB;
        }
	else
	  ch = 0;  // unhandled operator; skip char
	     
        if (ch) { 
	  if (!IsMonadic(ch)) {
	    if (!hs.empty()) {
	      last_op = hs.top();
	      byte next_to_last = 0;
	      if (hs.size() > 1) {
	        hs.pop();
	        next_to_last = hs.top(); // we need next to last
	        hs.push(last_op);
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
	        operands_pending = PushOperators(&hs, &op, operands_pending);
	      }
	    }
          }
	  hs.push(ch);
	  // cout << "Hold " << OpChar(ch) << endl;
	  // cout << "Held items = " << hs.size() << endl;
	}
	++cp;
      }
      else if ((f = strtod(cp, &endp)) != 0.) {
	// push a numeric constant operand into the opcode queue
	// cout << "Push " << f << endl;
        op.push_back(OP_FVAL);
        bp = (byte*) &f;
        for (i = 0; i < sizeof(double); i++)
          op.push_back(*(bp+i));         // store operand
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
          ecode = 2;  // unrecognized name
          goto endloop;
        }
        ++cp;
      }
      else if (*cp == '=') {
        while (op.size() > PREFIX_CODE_SIZE) {
          byte c = op.back();
          op.pop_back();
          // cout << "Pop " << OpChar(c) << endl;
        }
        operands_pending = 0;
	while (!hs.empty()) hs.pop();
        ++cp;
	tp = cp;
      }
      else                    // error in rescale expression
      {
         temp = *cp;         // for debug use
         ecode = 1;
         goto endloop;
      }
   }

endloop:

    if (! final_op) ecode = 2;   // error: no destination for rescale
    // cout << "Held items: " << hs.size() << endl;
    while (hs.size()) {
      last_op = hs.top();
      op.push_back(last_op);
      hs.pop();
      // cout << "Push " << OpChar(last_op) << endl;
    }

    if (ecode == 0)
    {
        op.push_back(OP_POP);
        op.push_back(OP_DUP);
        op.push_back(OP_2SWAP);
        op.push_back(OP_ROT);

        if (final_op == 1)
        {
            ;
        }
        else if (final_op == 2)
        {
	    op.push_back(OP_CELLPLUS);
        }
        else
        {
            ;
        }
        op.push_back(OP_SFSTORE);
        op.push_back(OP_RFETCH);
        op.push_back(OP_ADD);   // advance data ptr by nbytes
        op.push_back(OP_POP);
        op.push_back(OP_LOOP);
        op.push_back(OP_2DROP); // clean up the stack
        op.push_back(OP_RET); // set RET opcode to indicate end of instructions
    }

    // Copy the opcode sequences into the destination vector

    for (i = 0; i < op.size(); i++) pOpCodes->push_back(op[i]);

    return ecode;
}

char OpChar(byte op)
{
	char c;
	switch (op) {
	  case OP_FADD:
	    c = '+';
	    break;
	  case OP_FSUB:
	    c = '-';
	    break;
	  case OP_FMUL:
	    c = '*';
	    break;
	  case OP_FDIV:
	    c = '/';
	    break;
	  case OP_FPOW:
	    c = '^';
	    break;
	  case OP_FNEG:
	    c = 'N';
	    break;
	  default:
	    c = '?';
	    break;
	}
	return( c );
}

bool IsMonadic (byte opval)
{
    // Return true if byte code is for a monadic operator
    bool flag = false;
    switch (opval) {
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

int PushOperators ( stack<byte>* pH, deque<byte>* pO, int pending )
{
    // Push operators from hold stack into the opcode queue,
    // if there are sufficient operands available.
    byte last_op;
    while ( (!pH->empty()) && pending) {
      last_op = pH->top();		
      if (IsMonadic(last_op)) {
        pO->push_back(last_op);
      }
      else if (pending > 1) {
        pO->push_back(last_op);
        --pending;
      }
      else
       break;

      // cout << "Push " << OpChar(last_op) << endl;
      pH->pop();
    }
    return pending;
}

