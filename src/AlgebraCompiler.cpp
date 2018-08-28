// AlgebraCompiler.cpp
//
// Copyright (c) 1998--2018 Krishna Myneni 
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU General Public License (GPL), v3.0 or later.
//
#include <vector>
#include <stack>
#include <deque>
using std::vector;
using std::deque;
using std::stack;
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "fbc.h"
#define byte unsigned char

int CompileAE (vector<byte>*, char* exp);

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

    char* delim = " ,\t\n\r";
    char* arm_op = "^*/+-";

    // stack< vector<byte> > hs;    // operator hold stack
    stack<byte> hs;             // operator hold stack
    deque<byte> op;             // sequence of operators in double ended queue

    char exp_copy [256];
    char LabelName [256];
    int i, ival, ecode = 0;
    char *tp, *cp, *endp;
    byte last_op = 0, final_op = 0, temp, ch, *bp;
    double f;

    strcpy (exp_copy, exp);
    tp = strtok (exp_copy, delim);      // ptr to token

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

    do
    {
        cp = tp;    // set character ptr to first char in token

        while (*cp)
        {

            if ((*cp == '+') || (*cp == '-'))
            {
                // Is it a sign operator or an arithmetic operator?

                if (hs.empty() && (op.size() > 18))
                {
                    // It's an arithmetic operator; push onto hold stack

                    if (*cp == '+') ch = OP_FADD; else ch = OP_FSUB;
                    hs.push(ch);
                    last_op = ch;
                }
                else if (*cp == '-')
                {
                    // It's a sign operator; push negate operator onto
                    //   the hold stack

                    hs.push(OP_FNEG);
                }
                else
                    ;
                ++cp;   // increment the character pointer

            }
            else if ((f = strtod(cp, &endp)) != 0.)
            {
                op.push_back(OP_FVAL);
                bp = (byte*) &f;
                for (i = 0; i < sizeof(double); i++)
                    op.push_back(*(bp+i));         // store operand


                // pop all operators from hold stack into the opcode array

                while (!hs.empty())
                {
                    op.push_back(hs.top());
                    hs.pop();
                }

                cp = endp;          // advance character ptr
            }
            else if (isalpha(*cp))
            {

                if (*cp == 'X')     // fetch x
                {
                    op.push_back(OP_RFETCH);
                    op.push_back(OP_SFFETCH);
                    if (! final_op) final_op = 1;

                    // pop all operators from hold stack

                    while (!hs.empty())
                    {
                        op.push_back(hs.top());
                        hs.pop();
                    }

                }
                else if (*cp == 'Y')   // fetch y
                {
                    op.push_back(OP_RFETCH);
                    op.push_back(OP_IVAL);
                    ival = 4;
                    bp = (byte*)&ival;
                    for (i = 0; i < sizeof(int); i++)
                        op.push_back(*(bp + i));
                    op.push_back(OP_ADD);
                    op.push_back(OP_SFFETCH);
                    if (! final_op) final_op = 2;

                    // pop all operators from hold stack

                    while (!hs.empty())
                    {
                        op.push_back(hs.top());
                        hs.pop();
                    }

                }
                // else if (IsOperator(LabelName, &temp))
                // {
                    // push function operator onto the hold stack

                   // hs.push(temp);
                // }
                else
                {
                    ecode = 2;  // unrecognized name
                    goto endloop;
                }
                ++cp;
            }
            else if (*cp == '=')
            {
                while (op.size() > 18)
                {
                    op.pop_back();
                }
                ++cp;

            }
            else if (strchr (arm_op, *cp))  // arithmetic operation
            {
                // It's an arithmetic operator; check last opcode to see
                //   if it's an arithmetic operator of lower precedence.
                //   If so push both operators onto hold stack in the
                //   appropriate order.

                ch = *cp;
                if (ch == '^')
                    ch = OP_FPOW;
                else if (ch == '*')
                    ch = OP_FMUL;
                else if (ch == '/')
                    ch = OP_FDIV;
                else if (ch == '+')
                    ch = OP_FADD;
                else
                    ch = OP_FSUB;

                if (
                    (last_op != 0) &&
                    ((ch == OP_FPOW) && (last_op != OP_FPOW)) ||
                     (((ch == OP_FMUL) || (ch == OP_FDIV)) &&
                     ((last_op == OP_FADD) || (last_op == OP_FSUB)))
                   )
                {
                    hs.push(last_op);
                    hs.push(ch);
                    op.pop_back();
                }
                else
                {
                    hs.push(ch);
                }
                last_op = ch;
                ++cp;
            }
            else                    // error in rescale expression
            {
                temp = *cp;         // for debug use
                ecode = 1;
                goto endloop;
            }

        }

        tp = strtok (NULL, delim);  // get next token
    } while (tp) ;

endloop:

    if (! final_op) ecode = 2;   // error: no destination for rescale

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
            op.push_back(OP_IVAL);
            ival = 4;
            bp = (byte*) &ival;
            for (i = 0; i < sizeof(int); i++)
                op.push_back(*(bp + i));
            op.push_back(OP_ADD);
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











