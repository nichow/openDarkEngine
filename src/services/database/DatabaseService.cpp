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
 *****************************************************************************/

#include "ServiceCommon.h"
#include "DatabaseService.h"
#include "OpdeException.h"
#include "logger.h"

#include <OgreResourceGroupManager.h>

using namespace std;
using namespace Ogre;

namespace Opde {

	/*--------------------------------------------------------*/
	/*-------------------- DatabaseService -------------------*/
	/*--------------------------------------------------------*/
	DatabaseService::DatabaseService(ServiceManager *manager) : Service(manager), mCurDB(NULL) {

	}

    //------------------------------------------------------
    void DatabaseService::init() {
        // Create all services listening to the database messages...
	    ServiceManager::getSingleton().createByMask(SERVICE_DATABASE_LISTENER);
    }

	//------------------------------------------------------
	DatabaseService::~DatabaseService() {
	}

	//------------------------------------------------------
	void DatabaseService::load(const std::string& filename) {
	    LOG_DEBUG("DatabaseService::load - Loading requested file %s", filename.c_str());
        // if there is another database in hold
		if (!mCurDB.isNull())
			unload();

	    // Try to find the database
		mCurDB = getDBFileNamed(filename);

		// Currently hardcoded to mission db
		_loadMissionDB(mCurDB);

		LOG_DEBUG("DatabaseService::load - end()");
	}

	//------------------------------------------------------
	FileGroupPtr DatabaseService::getDBFileNamed(const std::string& filename) {
	    // TODO: Group of of the resource through the configuration service, once written
	    Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(filename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
		FilePtr fp = new OgreFile(stream);

		return new DarkFileGroup(fp);
	}

	//------------------------------------------------------
	void DatabaseService::unload() {
	    LOG_DEBUG("DatabaseService::unload");
	    DatabaseChangeMsg m;

	    m.change = DBC_DROPPING;
        m.dbtype = DBT_COMPLETE;
        m.dbtarget = DBT_COMPLETE; // No meaning here, at least now.
        // ^ It could be used to unload f.e. only mission or savegame, thus saving reload. This is a nice subject to think about, yes

        m.db = mCurDB;

        broadcastMessage(m);

		/// Wipe out the files we used
		mCurDB.setNull();

		LOG_DEBUG("DatabaseService::unload - end()");
	}

	//------------------------------------------------------
	void DatabaseService::_loadMissionDB(FileGroupPtr db) {
	    LOG_DEBUG("DatabaseService::_loadMissionDB");

		_loadGameSysDB(db);

		// Load the Mission
        // Create a DB change message, and broadcast
        DatabaseChangeMsg m;

        m.change = DBC_LOADING;
        m.dbtype = DBT_MISSION;
        m.dbtarget = DBT_MISSION; // TODO: Hardcoded
        m.db = db;

        broadcastMessage(m);
	}

	//------------------------------------------------------
	void DatabaseService::_loadGameSysDB(FileGroupPtr db) {
	    LOG_DEBUG("DatabaseService::_loadGameSysDB");

		// GAM_FILE
		FilePtr fdm = db->getFile("GAM_FILE");

		char* data = new char[fdm->size()];

		data[0] = 0x0;

		fdm->read(data, fdm->size());

		FileGroupPtr gs = getDBFileNamed(data);

        // Create a DB change message, and broadcast
        DatabaseChangeMsg m;

        m.change = DBC_LOADING;
        m.dbtype = DBT_GAMESYS;
        m.dbtarget = DBT_MISSION; // TODO: Hardcoded
        m.db = gs;

        broadcastMessage(m);
	}


	//-------------------------- Factory implementation
	std::string DatabaseServiceFactory::mName = "DatabaseService";

	DatabaseServiceFactory::DatabaseServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	const std::string& DatabaseServiceFactory::getName() {
		return mName;
	}

	Service* DatabaseServiceFactory::createInstance(ServiceManager* manager) {
		return new DatabaseService(manager);
	}

}