/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2007 openDarkEngine team
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/copyleft/lesser.txt.
 *****************************************************************************/
 
#ifndef __DTYPESCRIPTCOMPILER_H
#define __DTYPESCRIPTCOMPILER_H

#include <OgreCompiler2Pass.h>
#include <OgreDataStream.h>
#include <stack>
#include "DTypeDef.h"
#include "DVariant.h"
#include "BinaryService.h"

namespace Opde {

	/** DType (Dynamic typedef) script compiler. Compiles the C-like syntax (sort-of) from the given DataStream, and fills the BinaryService, with the resulting structures.
	* the syntax of such scripts is this: <code><br>
	<B>enum definition</B> <br>
	enum EnumName : VariantType {<br>
		key Name Value<br>
		key "Name 2" "Value 2"<br>
		...etc...<br>
	}<br>
	<br>
	<B>VariantType is one of: <i>uint, int, float, bool, string, vector</i></B> <br>
	<br>
	<B>bitfield definition, simmilar to enum</B><br>
	bitfield Name {<br>
		...key definitions, the same as in enum...<br>
	}<br>
	<br>
	<B>Variable type definitions</B><br>
	<B>A previously defined type alias</B><br>
	alias Type NewType;<br>
	<B>struct / union definition</B><br>
	struct Str1 [ [len] ] {<br>
		...type definitions, structs, unions...<br>
	}<br>
	union Un1 {<br>
		...type definitions, structs, unions...<br>
	}<br>
	<br>
	Type [use Enum/Bitfield] name [ [array size] ] name [ = "default value string" ]<br>
	<br>
	<B>in case of type == char (fixed string), a box brace follows the char like this:</B><br>
	char [64] a_long_string;<br>
	<br>
	<B>Type is one of: <i>uintXX, intXX, float, boolXX, varstr, vector, char [len]</i></B> <br>
	<br>
	<B>varstr (variable length string) cannot be contained in a struct/union/array</B>
	</code>
	*/
	class DTypeScriptCompiler : public Ogre::Compiler2Pass {
		public:
			/** Compiler */
			DTypeScriptCompiler(void);
			
			/** Destructor */
			~DTypeScriptCompiler(void);
			
			/** gets BNF Grammer for dtype script.
			*/
			const Ogre::String& getClientBNFGrammer(void) const { return dtypeScript_BNF; }

			/** get the name of the BNF grammar.
			*/
			virtual const Ogre::String& getClientGrammerName(void) const { static const Ogre::String grammerName("DType Script"); return grammerName; }
			
			/** Parses the script and fills the BinaryService */
			void parseScript(Ogre::DataStreamPtr& stream, const Ogre::String& groupName) {
				Compiler2Pass::compile(stream->getAsString(),  stream->getName());
			}
			
			/** Returns the position usable for autogenerated tokens to use - ogre's requirement */
			virtual size_t getAutoTokenIDStart() const;

	protected:
		// Token ID enumeration
		enum TokenID {
			ID_UNKOWN = 0, 
			ID_OPENBRACE, ID_CLOSEBRACE, // { and }
			ID_OPENBOX, ID_CLOSEBOX, // [ and ]
			ID_NAMESPACE, ID_ENUM, ID_BITFIELD, ID_KEY, 
			ID_ALIAS, ID_STRUCT, ID_UNION, 
			ID_EQUALS, // =
			ID_USE, // 'use' keyword for enumeration/bitfield
			ID_INT,
			ID_UINT,
			// Primitive Types
			ID_INT32, ID_INT16, ID_INT8,
			ID_UINT32, ID_UINT16, ID_UINT8,
			ID_BOOL32, ID_BOOL16, ID_BOOL8,
			ID_FLOAT,
			ID_STRING, // String as a dvariant type
			ID_CHAR, // String with defined size
			ID_VARSTR, // Variable length string
			ID_VECTOR,
			ID_SHORTVECTOR,
			ID_AUTOTOKEN // The start of the autogenerated tokens
		};
	
		/** ID of the current structure */
		enum CompileStateID {
			/// Unknown position
			CS_UNKNOWN = 0, 
			/// In namespace
			CS_NAMESPACE, 
			/// In enum definition
			CS_ENUM, 
			/// In struct definition
			CS_STRUCT
		};
		
		/** Compilation state. The position the compilation is in, together with some information about the context. */
		typedef struct CompileState {
			CompileStateID state;
			bool unioned; // true if the created struct should be unioned
			DTypeDefVector types; // for struct, list of members.
			DEnumPtr enumeration;
			DVariant::Type enumvaltype;
			size_t arraylen;
			std::string name;
		};
		
		/** The stack of the current position (Namespace->struct->struct for example) */
		typedef std::stack<CompileState> CompileStateStack;
		
		/// Stack of the compiler states
		CompileStateStack mStateStack;
		
		/// The current state
		CompileState mCurrentState;
		
		/// BinaryService the compiler fills with the result
		BinaryServicePtr mBinaryService;
		
		/** Pop a state from the state stack 
		* @return Previous current state */
		CompileState popState();
		
		/** Push the current state to the state stack top, and set the current to CS_UNKNOWN */
		void pushCurrentState();
		
		/** get the variant type from string type specifier */
		DVariant::Type getDVTypeFromStr(const std::string& name);
				
		/** Convert to DVariant::Type from TokenID, or logParseError if unsuccessfull */
		DVariant::Type getDVTypeFromID(TokenID id);
		
		/** Get the data length from TokenID */
		int getDataLenFromID(TokenID id);
				
		/** get the typedef from binary service. First the namespace's, then the global if not found in namespace */
		DTypeDefPtr getTypeDef(const std::string& name);
		
		/** get the enumeration from binary service. First the namespace's, then the global if not found in namespace */
		DEnumPtr getEnum(const std::string& name);
		
		/** Dispatches the type definition according to the current state */
		void dispatchType(DTypeDefPtr& def);

		/** Expects, and reads the number of elements in the [] parenthesis. Returns the parsed number */
		int parseBoxBrace();
		
		/// Type definition group name. Namespace that the current parsed code is in
		std::string mGroupName;

		/// static library database for tokens and BNF rules
		static TokenRule dtypeScript_RulePath[];
		
		/// simplified Backus - Naur Form (BNF) grammer for the dtype script
		static Ogre::String dtypeScript_BNF;

		/// CallBack funcition type
		typedef void (DTypeScriptCompiler::* DTS_Action)(void);
		
		/// Map of token ID->method callback
		typedef std::map<size_t, DTS_Action> TokenActionMap;
		
		/// Iterator over TokenActionMap
		typedef TokenActionMap::iterator TokenActionIterator;
		
		/** Feed the compiler with the token definitions */
        	virtual void setupTokenDefinitions(void);
		
		/** Register a token, with possible action if not NULL */
		void addLexemeTokenAction(const Ogre::String& lexeme, const size_t token, const DTS_Action action = 0);
		
		/** Execute the action given the token ID */
		void executeTokenAction(const size_t tokenID);
		
		/** If something goes wrong, this method is called */
		void logParseError(const Ogre::String& error);
		
		/** Map of Token value as key to an Action.  An Action converts tokens into
		    the final format.
		    All instances use the same Token Action Map.
		*/
		static TokenActionMap mTokenActionMap;
		
		// ---------- Parsing methods -----------------------
		/// Parses a Opening Curly Brace
		void parseOpenBrace(void);
		
		/// Parses a Closing Curly Brace
		void parseCloseBrace(void);
		
		/// Parses a namespace definition
		void parseNameSpace(void);
		
		/// Parses an enumeration definition (or bitfield)
		void parseEnum(void);
		
		/// Parses a single enum key
		void parseEnumKey(void);
		
		/// Parses a plain (not-in struct) alias definition
		void parseAlias(void);
		
		/// Parses a struct/union definition
		void parseStruct(void);
		
		/// Parses a field definition
		void parseField(void);
	};
}

#endif

