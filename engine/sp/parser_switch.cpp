/*
   FALCON - The Falcon Programming Language.
   FILE: parser_sitch.cpp

   Parser for Falcon source files -- namespace directive
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sat, 19 May 2012 20:00:24 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#undef SRC
#define SRC "engine/sp/parser_switch.cpp"

#include <falcon/error.h>
#include <falcon/synclasses_id.h>
#include <falcon/sp/sourceparser.h>
#include <falcon/sp/parsercontext.h>
#include <falcon/sp/parser_atom.h>
#include <falcon/sp/parser_deletor.h>

#include <falcon/parser/rule.h>
#include <falcon/parser/parser.h>

#include <falcon/sp/sourcelexer.h>
#include <falcon/sp/parser_switch.h>

#include <falcon/psteps/stmtselect.h>
#include <falcon/psteps/stmtswitch.h>
#include <falcon/symbol.h>

#include "private_caselist.h"

#include <deque>

namespace Falcon {

using namespace Parsing;


bool switch_errhand(const NonTerminal&, Parser& p)
{
   //SourceParser* sp = static_cast<SourceParser*>(p);
   TokenInstance* ti = p.getNextToken();
   p.addError( e_switch_decl, p.currentSource(), ti->line(), ti->chr() );

   // remove the whole line
   p.consumeUpTo( p.T_EOL );
   p.clearFrames();
   return true;
}


static void on_switch_closed(void* parser_void)
{
   Parser* p = static_cast<Parser*>(parser_void);
   ParserContext* ctx = static_cast<ParserContext*>(p->context());
   TreeStep* stmt = ctx->currentStmt();
   fassert( stmt != 0 );
   fassert( (stmt->handler()->userFlags() == FALCON_SYNCLASS_ID_CASEHOST)
            || (stmt->handler()->userFlags() == FALCON_SYNCLASS_ID_SWITCH) );
   SwitchlikeStatement* swc = static_cast<SwitchlikeStatement*>( stmt );
   
   if( swc->dummyTree() != 0 ) {
      SynTree* dummytree = swc->dummyTree();
      for( uint32 i = 0; i < dummytree->size(); i++ ) {
         // ops, statements were added outside the the case frame.
         Statement* child = static_cast<Statement*>(dummytree->at(i));
         p->addError( e_switch_body, p->currentSource(), child->line(), child->chr() );
      }
   }
}


void apply_switch( const Rule&, Parser& p )
{
   // << T_switch << Expr << T_EOL
   TokenInstance* tswch = p.getNextToken();
   TokenInstance* texpr = p.getNextToken();

   ParserContext* st = static_cast<ParserContext*>(p.context());

   Expression* swexpr = static_cast<Expression*>(texpr->detachValue());
   st->accessSymbols(swexpr);
   StmtSwitch* stmt_switch = new StmtSwitch( swexpr, tswch->line(), tswch->chr() );
   
   // clear the stack
   p.simplify(3);

   st->openBlock( stmt_switch, stmt_switch->dummyTree() );
   p.pushState("InlineFunc", &on_switch_closed, &p);   
}


void apply_select( const Rule&, Parser& p )
{
   // << T_select << Expr << T_EOL
   TokenInstance* tswch = p.getNextToken();
   TokenInstance* texpr = p.getNextToken();

   ParserContext* st = static_cast<ParserContext*>(p.context());

   Expression* swexpr = static_cast<Expression*>(texpr->detachValue());
   st->accessSymbols(swexpr);
   StmtSelect* stmt_sel = new StmtSelect( swexpr, tswch->line(), tswch->chr() );

   // clear the stack
   p.simplify(3);

   st->openBlock( stmt_sel, stmt_sel->dummyTree() );
   p.pushState( "InlineFunc", &on_switch_closed, &p);

}


static bool make_case_branch(  Parser& p, ParserContext* ctx, SynTree* st, bool bOpenBranch )
{
   // << T_case << CaseList << T_COLON
   // << T_case << CaseList << T_EOL
   p.getNextToken();
   TokenInstance* tlist = p.getNextToken();      
   
   // get a new syntree for what comes next.
   TreeStep* stmt = ctx->currentStmt();
   if( stmt == 0 ) {
       p.addError(e_case_outside, p.currentSource(), tlist->line(), tlist->chr() );
       return false;
   }
      
   CaseList* cli = static_cast<CaseList*>(tlist->asData());
   Class* handler = stmt->handler();
   
   // is this a switch or a select?
   if ( handler->userFlags() == FALCON_SYNCLASS_ID_SWITCH )
   {
      StmtSwitch* swc = static_cast<StmtSwitch*>(stmt);

      CaseList::iterator iter = cli->begin();
      while( iter != cli->end() ) {
         CaseItem* itm = *iter;
         bool noClash;

         switch( itm->m_type ) {
            case CaseItem::e_nil: noClash = swc->addNilBlock( st ); break;
            case CaseItem::e_true: noClash = swc->addBoolBlock( true, st ); break;
            case CaseItem::e_false: noClash = swc->addBoolBlock( false, st ); break;
            case CaseItem::e_int: noClash = swc->addIntBlock( itm->m_iLow, st ); break;
            case CaseItem::e_string: noClash = swc->addStringBlock( *itm->m_sLow, st ); break;
            case CaseItem::e_sym:
               noClash = swc->addSymbolBlock( itm->m_sym, st );
               break;
            case CaseItem::e_rngInt: noClash = swc->addRangeBlock( 
                                       itm->m_iLow, itm->m_iHigh, st ); break;
            case CaseItem::e_rngString: noClash = swc->addStringRangeBlock( 
                                 *itm->m_sLow, *itm->m_sHigh, st ); break;
         }

         if ( ! noClash ) {
            p.addError(e_switch_clash, p.currentSource(), tlist->line(), tlist->chr() );
            return false;
         }
         ++iter;
      }
      
      if (bOpenBranch) ctx->openTempBlock( swc->dummyTree(), st );
   }
   else if( handler->userFlags() == FALCON_SYNCLASS_ID_CASEHOST )
   {
      StmtSelect* swc = static_cast<StmtSelect*>(stmt);

      CaseList::iterator iter = cli->begin();
      while( iter != cli->end() ) {
         CaseItem* itm = *iter;
         bool noClash;

         switch( itm->m_type ) {
            case CaseItem::e_int: noClash = swc->addSelectType( itm->m_iLow, st ); break;
            case CaseItem::e_sym:
               // Search the symbol
               {
                  Variable* symBaseClass = ctx->accessSymbol( itm->m_sym->name() );
                  // if it's 0, we need a requirement; and we shall also add an external symbol.
                  if( symBaseClass == 0 || symBaseClass->type() == Variable::e_nt_extern )
                  {
                     // then just add the requirement
                     if( p.interactive() )
                     {
                        p.addError( e_undef_sym, p.currentSource(), tlist->line(), tlist->chr(), 0, itm->m_sym->name() );
                        return false;
                     }

                     Requirement* req = swc->addSelectName( itm->m_sym->name(), st );
                     if( req == 0 )
                     {
                        noClash = false;
                     }
                     else {
                        ctx->onRequirement( req );
                     }
                  }
                  else
                  {
                     // if it's defined and not a class, we're in trouble
                     const Item* value = ctx->getVariableValue( itm->m_sym->name(), symBaseClass );
                     fassert( value != 0 );

                     if( value == 0 || ! value->isClass() )
                     {
                        p.addError( e_select_invtype, p.currentSource(), tlist->line(), tlist->chr() );
                        return false;
                     }
                     else {
                        // cool, we can add the resolved symbol
                        noClash = swc->addSelectClass( static_cast<Class*>(value->asInst()), st );
                     }
                  }
               }
               break;
            
            case CaseItem::e_string:
            case CaseItem::e_nil: 
            case CaseItem::e_true:
            case CaseItem::e_false:
            case CaseItem::e_rngInt:
            case CaseItem::e_rngString: 
               p.addError(e_select_decl, p.currentSource(), tlist->line(), tlist->chr() );
               break;
         }

         if ( ! noClash ) {
            p.addError(e_switch_clash, p.currentSource(), tlist->line(), tlist->chr() );
            return false;
         }
         ++iter;
      }
      
      if (bOpenBranch) ctx->openTempBlock( swc->dummyTree(), st );  
   }
   else {
      p.addError(e_case_outside, p.currentSource(), tlist->line(), tlist->chr() );
      if (bOpenBranch) ctx->openTempBlock( 0, st );  
      return false;
   }
   
   return true;
}


static bool make_default_branch( Parser& p, ParserContext* ctx, SynTree* st, bool bOpenBranch )
{
   // the caller didn't take any token.
   TokenInstance* ti = p.getNextToken();
      
   // Gets the parent statement.
   TreeStep* stmt = ctx->currentStmt();
   if( stmt == 0 ) {
       p.addError(e_case_outside, p.currentSource(), ti->line(), ti->chr() );
       return false;
   }
   
   
   Class* handler = stmt->handler();
   
   // is this a switch or a select?
   bool bClash;
   if ( handler->userFlags() == FALCON_SYNCLASS_ID_SWITCH )
   {
      StmtSwitch* swc = static_cast<StmtSwitch*>(stmt);
      bClash = swc->setDefault( st );
      if (bOpenBranch) ctx->openTempBlock( swc->dummyTree(), st );  
   }
   else if( handler->userFlags() == FALCON_SYNCLASS_ID_CASEHOST )
   {
      StmtSelect* swc = static_cast<StmtSelect*>(stmt);
      bClash = swc->setDefault( st );
      if (bOpenBranch) ctx->openTempBlock( swc->dummyTree(), st );  
   }
   else {
      if (bOpenBranch) ctx->openTempBlock( 0, st );  
      p.addError(e_case_outside, p.currentSource(), ti->line(), ti->chr() );
      return false;
   }
   
   if ( ! bClash ) {
      p.addError(e_switch_default, p.currentSource(), ti->line(), ti->chr() );
      return false;
   }
   
   return true;
}

void apply_case( const Rule&, Parser& p )
{   
   // the current statement is a switch.
   ParserContext* ctx = static_cast<ParserContext*>(p.context());
   SynTree* st = ctx->changeBranch();    
   if( st != 0 )
   {
      make_case_branch( p, ctx, st, false );
   }
      
   // clear the stack
   p.simplify(3);
}


void apply_case_short( const Rule&, Parser& p )
{
  
   // the current statement is a switch.
   ParserContext* ctx = static_cast<ParserContext*>(p.context());
   SynTree* st = new SynTree;
   make_case_branch( p, ctx, st, true );  
   
   // clear the stack
   p.simplify(3);
}



void apply_default( const Rule&, Parser& p )
{
   // << T_default << T_EOL   
   ParserContext* ctx = static_cast<ParserContext*>(p.context());
   SynTree* st = ctx->changeBranch(); 
   make_default_branch( p, ctx, st, false );
   
   p.simplify(2);
}


void apply_default_short( const Rule&, Parser& p )
{
   // << T_default << T_COLON
   ParserContext* ctx = static_cast<ParserContext*>(p.context());
   SynTree* st = new SynTree;
   make_default_branch( p, ctx, st, true );  
   
   p.simplify(2);
   
}

void apply_CaseListRange_int( const Rule&, Parser& p )
{
   // << T_Int << T_to << T_Int );
   TokenInstance* ti1 = p.getNextToken();
   p.getNextToken();
   TokenInstance* ti2 = p.getNextToken();
   
   SourceParser* sp = static_cast<SourceParser*>(&p);
   TokenInstance* tir = TokenInstance::alloc(ti1->line(), ti1->chr(), sp->CaseListRange );   
   tir->setValue( new CaseItem( ti1->asInteger(), ti2->asInteger() ), CaseItem::deletor );
   
   p.simplify(3,tir);
}

void apply_CaseListRange_string( const Rule&, Parser& p )
{
   // << T_String << T_to << T_String );
   TokenInstance* ti1 = p.getNextToken();
   p.getNextToken();
   TokenInstance* ti2 = p.getNextToken();
   
   SourceParser* sp = static_cast<SourceParser*>(&p);
   TokenInstance* tir = TokenInstance::alloc(ti1->line(), ti1->chr(), sp->CaseListRange );   
   tir->setValue( new CaseItem( ti1->detachString(), ti2->detachString() ), CaseItem::deletor );
   
   p.simplify(3,tir);
}

void apply_CaseListToken_range( const Rule&, Parser& p )
{
   TokenInstance* ti = p.getNextToken();
   // we just have to change the token type.
   ti->token( static_cast<SourceParser*>(&p)->CaseListToken );
}

void apply_CaseListToken_nil( const Rule&, Parser& p )
{
   //<< T_nil 
   TokenInstance* ti = p.getNextToken();
   ti->token( static_cast<SourceParser*>(&p)->CaseListToken );
   ti->setValue( new CaseItem(), CaseItem::deletor );   
}

void apply_CaseListToken_true( const Rule&, Parser& p )
{
   //<< T_true
   TokenInstance* ti = p.getNextToken();
   ti->token( static_cast<SourceParser*>(&p)->CaseListToken );
   ti->setValue( new CaseItem( true ), CaseItem::deletor );   
}

void apply_CaseListToken_false( const Rule&, Parser& p )
{
   //<< T_false
   TokenInstance* ti = p.getNextToken();
   ti->token( static_cast<SourceParser*>(&p)->CaseListToken );
   ti->setValue( new CaseItem( false ), CaseItem::deletor );   
}

void apply_CaseListToken_int( const Rule&, Parser& p )
{
   //<< T_true
   TokenInstance* ti = p.getNextToken();
   ti->token( static_cast<SourceParser*>(&p)->CaseListToken );
   ti->setValue( new CaseItem( ti->asInteger() ), CaseItem::deletor );   
}

void apply_CaseListToken_string( const Rule&, Parser& p )
{
   //<< T_true
   TokenInstance* ti = p.getNextToken();
   ti->token( static_cast<SourceParser*>(&p)->CaseListToken );
   ti->setValue( new CaseItem( ti->detachString() ), CaseItem::deletor );
}

void apply_CaseListToken_sym( const Rule&, Parser& p )
{
   TokenInstance* ti = p.getNextToken();
   SourceParser* sp = static_cast<SourceParser*>(&p);
   ParserContext* ctx = static_cast<ParserContext*>(p.context());

   String& name = *ti->asString();
   ctx->accessSymbol(name);
   // SYM is not 0
   ti->token( sp->CaseListToken );
   ti->setValue( new CaseItem( Engine::getSymbol(name) ), &CaseItem::deletor );
}


void apply_CaseList_next( const Rule&, Parser& p )
{
   // << CaseList << T_Comma << CaseListToken
   TokenInstance* til = p.getNextToken();
   p.getNextToken();
   TokenInstance* tit = p.getNextToken();
   
   CaseList* lst = static_cast<CaseList*>(til->asData());
   CaseItem* itm = static_cast<CaseItem*>(tit->detachValue());
   lst->push_back(itm);
   p.trimFromBase(1,2);
}


void apply_CaseList_first( const Rule&, Parser& p )
{
   // << CaseListToken
   TokenInstance* tit = p.getNextToken();
   SourceParser* sp = static_cast<SourceParser*>(&p);
   
   CaseItem* itm = static_cast<CaseItem*>(tit->detachValue());
   CaseList* lst = new CaseList;
   lst->push_back( itm );
   
   tit->token( sp->CaseList );
   tit->setValue( lst, &CaseList::deletor );
}

}

/* end of parser_switch.cpp */