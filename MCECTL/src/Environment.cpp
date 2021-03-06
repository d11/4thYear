/*
 * =====================================================================================
 *
 *       Filename:  Environment.cpp
 *    Description:  Definitions for the Environment class
 *
 * =====================================================================================
 */

#include <map>
#include <numeric>

#include "formula/Formula.h"
#include "Environment.h"
#include "ModelChecker.h"
#include "Automata.h"
#include "TransitionSystem.h"
#include "exception/NonExistentFormula.h"
#include "exception/AlreadyExistsException.h"
#include "Util.h"

Environment::Environment() {

   // Create special automata (for AX, EX)
   State *s1 = new State("s1", false); // initial
   State *s2 = new State("s2", true);  // accepting
   vector<State*> states;
   states.push_back(s1);
   states.push_back(s2);
   vector<string> names;
   names.push_back("s1"); 
   names.push_back("s2");
   vector<string> symbols;
   symbols.push_back("_");
   ConfigurationSpace *config_space = new ConfigurationSpace(names, symbols);
   DFA *dfa = new DFA(states, s1, config_space);
   RegularAction *action = new RegularAction(".", 1);
   dfa->AddRule(0, action);
   SetDFA("#X#", dfa);

}

const ResultsTable &Environment::GetCheckResultsByID(unsigned int system_id)  {
   map<unsigned int, ResultsTable *>::const_iterator iter;
   iter = _computed_results.find(system_id);
   ResultsTable *table = NULL;
   if (iter == _computed_results.end()) {
      table = new ResultsTable(*this);
      _computed_results.insert(make_pair(system_id, table));
   }
   else {
      table = iter->second;
   }
   return *table;
}
const ResultsTable &Environment::GetCheckResults(const KripkeStructure *transition_system)  {
	return GetCheckResultsByID(transition_system->GetID());
}
const ResultsTable &Environment::GetCheckResults(const PushDownSystem *transition_system)  {
	return GetCheckResultsByID(transition_system->GetID());
}

void Environment::SetCheckResultsByID(unsigned int system_id, Formula::Formula::const_reference formula, CheckResults *results) {
   map<unsigned int, ResultsTable *>::iterator iter;
   iter = _computed_results.find( system_id );
   ResultsTable *table = NULL;
   if (iter == _computed_results.end()) {
      table = new ResultsTable(*this);
      _computed_results.insert(make_pair(system_id, table));
   }
   else {
      table = iter->second;
   }
   table->SetEntry( formula, results );
}
void Environment::SetCheckResults(
      const KripkeStructure *transition_system,
      Formula::Formula::const_reference formula,
      CheckResults *results
   )
{
   SetCheckResultsByID( transition_system->GetID(), formula, results );
}
void Environment::SetCheckResults(
      const PushDownSystem *transition_system,
      Formula::Formula::const_reference formula,
      CheckResults *results
   ) 
{
   SetCheckResultsByID( transition_system->GetID(), formula, results );
}

Formula::Formula::const_reference Environment::GetFormula(const string &identifier) const {
   map<string, const Formula::Formula*>::const_iterator iter(_formulas.find(identifier));
   if (iter == _formulas.end()) {
      throw NonExistentFormulaException(identifier);
   }
   return *(iter->second);
};

Formula::Formula::const_reference Environment::GetFormulaByID(unsigned int formula_id) const {
   map<unsigned int, const Formula::Formula*>::const_iterator iter(_formulas_by_id.find(formula_id));
   if (iter == _formulas_by_id.end()) {
      throw NonExistentFormulaException("Couldn't get formula by ID!");
   }
   return *(iter->second);
};

void Environment::SetFormula( const string &identifier, Formula::Formula::const_reference formula ) {
   cout << "[Formula: "  << identifier << "]" << endl;
   bool result = _formulas.insert(make_pair(identifier, &formula)).second;
   if (!result) { throw AlreadyExistsException(identifier); }
	formula.Accept(*this);
}

void Environment::SetFormulaByID( Formula::Formula::const_reference formula ) {
   bool result = _formulas_by_id.insert(make_pair(formula.GetID(), &formula)).second;
   if (!result) {
		cout << "Warning: formula with id " << formula.GetID() << " already exists!" << endl; 
	}
}

void Environment::Visit(const Formula::False       &formula_false) {
	SetFormulaByID(formula_false);
}
void Environment::Visit(const Formula::True        &formula_true) {
	SetFormulaByID(formula_true);
}

void Environment::Visit(const Formula::PVar        &release) {
	SetFormulaByID(release);
}

void Environment::Visit(const Formula::Negation    &negation) {
	SetFormulaByID(negation);
	negation.GetSubFormula().Accept(*this);
}

void Environment::Visit(const Formula::Conjunction &conjunction) {
	SetFormulaByID(conjunction);
	conjunction.GetLeft().Accept(*this);
	conjunction.GetRight().Accept(*this);
}

void Environment::Visit(const Formula::Until       &until) {
	SetFormulaByID(until);
	until.GetBefore().Accept(*this);
	until.GetAfter().Accept(*this);
}

void Environment::Visit(const Formula::Release     &release) {
	SetFormulaByID(release);
	release.GetBefore().Accept(*this);
	release.GetAfter().Accept(*this);
}


string Environment::ToString() const {
   stringstream s;
   s << "Deterministic Finite Automata:" << endl
	  << accumulate(
           _dfas.begin(), _dfas.end(), string(""),
           JoinKeysWithSquareBrackets<string,const DFA*>
        )
     << endl;

   s << "Pushdown Automata:" << endl
	  << accumulate(
           _pdas.begin(), _pdas.end(), string(""),
			  JoinKeysWithSquareBrackets<string,const PDA*>
        )
     << endl;

   s << "Labelled Transition Systems:" << endl
	  << accumulate(
           _ltss.begin(), _ltss.end(), string(""),
			  JoinKeysWithSquareBrackets<string,const KripkeStructure*>
        )
     << endl;

   s << "Pushdown Systems:" << endl
	  << accumulate(
           _pdss.begin(), _pdss.end(), string(""),
			  JoinKeysWithSquareBrackets<string,const PushDownSystem*>
        )
     << endl;

   s << "Extended CTL Formulas:" << endl
	  << accumulate(
           _formulas.begin(), _formulas.end(), string(""),
			  JoinKeysWithSquareBrackets<string,const Formula::Formula*>
        )
     << endl;

   return s.str();
}

Environment::~Environment() {

   map<string, const DFA*>::const_iterator iter;
   for( iter = _dfas.begin(); iter != _dfas.end(); ++iter ) {
      delete iter->second;
   }
   map<string, const PDA*>::const_iterator iter_pda;
   for( iter_pda = _pdas.begin(); iter_pda != _pdas.end(); ++iter_pda ) {
      delete iter_pda->second;
   }
   map<string, const KripkeStructure*>::const_iterator iter_lts;
   for( iter_lts = _ltss.begin(); iter_lts != _ltss.end(); ++iter_lts ) {
      delete iter_lts->second;
   }
   map<string, const PushDownSystem*>::const_iterator iter_pds;
   for( iter_pds = _pdss.begin(); iter_pds != _pdss.end(); ++iter_pds ) {
      delete iter_pds->second;
   }
   map<string, const Formula::Formula*>::const_iterator formula_iter;
   for( formula_iter = _formulas.begin(); formula_iter != _formulas.end(); ++formula_iter ) {
      delete formula_iter->second;
   }
}
