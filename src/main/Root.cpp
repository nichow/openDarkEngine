/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *	  $Id$
 *
 *****************************************************************************/


#include "Root.h"
#include "OgreResourceGroupManager.h"

#ifdef CUSTOM_IMAGE_HOOKS
#include "CustomImageCodec.h"
#endif

namespace Opde {
	// -------------------------------------------------------
	Root::Root(uint serviceMask) : 
			mLogger(NULL), 
			mServiceMgr(NULL), 
			mOgreRoot(NULL), 
			mOgreLogManager(NULL), 
			mDTypeScriptCompiler(NULL),
			mPLDefScriptCompiler(NULL),
			mServiceMask(serviceMask) {
		
		mLogger = new Logger();
		
		// TODO: A method to create/enable logging. This setup currently does no logging at all
		mServiceMgr = new ServiceManager(mServiceMask);
		
		// To supress logging of OGRE (we'll use a plugin for our logger for Ogre logs)
		// we need to create a Ogre::LogManager here on our own
		mOgreLogManager = new Ogre::LogManager();
		mOgreLogManager->createLog("Ogre.log", true, true, true);
		
		mOgreOpdeLogConnector = new OgreOpdeLogConnector(mLogger);
		
		// create our logger's ogre log listener interface. Connect together
		mOgreLogManager->getDefaultLog()->addListener(mOgreOpdeLogConnector);
		
		mOgreRoot = new Ogre::Root();

		// if custom image hooks are to be included, setup now
#ifdef CUSTOM_IMAGE_HOOKS
		Ogre::CustomImageCodec::startup();
#endif
		// Now we need to register all the service factories
		registerServiceFactories();
		
		mDTypeScriptCompiler = new DTypeScriptCompiler();
		mPLDefScriptCompiler = new PLDefScriptCompiler();
	}
	
	// -------------------------------------------------------
	Root::~Root() {
		delete mDTypeScriptCompiler;
		delete mPLDefScriptCompiler;

		delete mServiceMgr;
		
#ifdef CUSTOM_IMAGE_HOOKS
		Ogre::CustomImageCodec::shutdown();
#endif
		delete mOgreRoot;
		
		// As the last thing - release the logger
		delete mLogger;
		
		delete mOgreOpdeLogConnector;
	}


	// -------------------------------------------------------
	void Root::bootstrapFinished() {
		mServiceMgr->bootstrapFinished();
	}

	// -------------------------------------------------------
	void Root::loadConfigFile(const std::string& fileName) {
		ConfigServicePtr cfp = mServiceMgr->getService("ConfigService").as<ConfigService>();
		
		cfp->loadParams(fileName);
	}

	// -------------------------------------------------------
	void Root::loadPLDefScript(const std::string& fileName, const std::string& groupName) {
		// try to open the given resource stream
		Ogre::DataStreamPtr str = Ogre::ResourceGroupManager::getSingleton().openResource(fileName, groupName, true, NULL);
		mPLDefScriptCompiler->parseScript(str, groupName);
	}

	// -------------------------------------------------------
	void Root::loadDTypeScript(const std::string& fileName, const std::string& groupName) {
		Ogre::DataStreamPtr str = Ogre::ResourceGroupManager::getSingleton().openResource(fileName, groupName, true, NULL);
		mDTypeScriptCompiler->parseScript(str, groupName);
	}

	// -------------------------------------------------------
	void Root::loadResourceConfig(const std::string& fileName) {
	    Ogre::ConfigFile cf;
		cf.load(fileName);

		// Go through all sections & settings in the file
		Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

		Ogre::String secName, typeName, archName;

		while (seci.hasMoreElements()) {
			secName = seci.peekNextKey();
			Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
			Ogre::ConfigFile::SettingsMultiMap::iterator i;

			for (i = settings->begin(); i != settings->end(); ++i) {
				typeName = i->first;
				archName = i->second;
				Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
					archName, typeName, secName);
			}
		}
	}
	
	// -------------------------------------------------------
	void Root::addResourceLocation(const std::string& archName, const std::string& typeName, const std::string& secName, bool recursive = false) {
		Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName, recursive);
	}
	
	// -------------------------------------------------------
	void Root::removeResourceLocation(const std::string& archName, const std::string& typeName, const std::string& secName) {
		Ogre::ResourceGroupManager::getSingleton().removeResourceLocation(archName, typeName, secName);
	}
	
	// -------------------------------------------------------
	void Root::registerServiceFactories() {
		// register all the service factories
		// The factories are deleted in service manager
		
		// TODO: WE ALL KNOW this is VERY WRONG way to work with memory allocation
		
		/* The right way would be for example:
		1. ServiceFactoryPtr - shared_ptr<ServiceFactory>;
		2. Work everywhere with the shared_ptr instead
		*/  
		
		new WorldRepServiceFactory();
		new BinaryServiceFactory();
		new GameServiceFactory();
		new ConfigServiceFactory();
		new LinkServiceFactory();
		new PropertyServiceFactory();
		new InheritServiceFactory();
		new RenderServiceFactory();
		new DatabaseServiceFactory();
		new InputServiceFactory();
		new LoopServiceFactory();
		new ObjectServiceFactory();
	}
}