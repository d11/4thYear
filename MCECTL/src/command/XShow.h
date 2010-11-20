/*
 * =====================================================================================
 *
 *       Filename:  XShow.h
 *    Description:  Show automata via graphviz
 *         Author:  Dan Horgan (danhgn), danhgn@googlemail.com
 *
 * =====================================================================================
 */

#ifndef  _XSHOW_H_
#define  _XSHOW_H_

#include <string>
#include <iostream>

#include "Command.h"
#include "Environment.h"

#include <stdio.h>

struct GlobalOptions;

namespace Command {

   class XShowCommand : public Command {
      private:
         string _identifier;
      public:
         XShowCommand(string identifier) : _identifier(identifier) { }
         virtual string ToString() const {
            string s = ":xshow(";
            return s + _identifier + ")" ;
         }
         virtual void Execute(Environment &environment, GlobalOptions &options) const {
				const Automaton *automaton(environment.GetAutomaton(_identifier));
				string s(automaton->ToDot());
	
				// TODO check return codes
				FILE *dot = popen("dot -Tx11", "w");
				if (NULL == dot) {
					throw runtime_error("Couldn't open pipe to 'dot' command");
				}
				fputs(s.c_str(), dot);
				pclose(dot);
         }
   };

}

#endif