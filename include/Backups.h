/* -------------------------------------------------------------------------- */
/* Copyright 2002-2022, OpenNebula Project, OpenNebula Systems                */
/*                                                                            */
/* Licensed under the Apache License, Version 2.0 (the "License"); you may    */
/* not use this file except in compliance with the License. You may obtain    */
/* a copy of the License at                                                   */
/*                                                                            */
/* http://www.apache.org/licenses/LICENSE-2.0                                 */
/*                                                                            */
/* Unless required by applicable law or agreed to in writing, software        */
/* distributed under the License is distributed on an "AS IS" BASIS,          */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   */
/* See the License for the specific language governing permissions and        */
/* limitations under the License.                                             */
/* -------------------------------------------------------------------------- */

#ifndef BACKUPS_H_
#define BACKUPS_H_

#include <string>
#include <map>

#include "ObjectCollection.h"
#include "Template.h"

class ObjectXML;

/**
 *  This class represents the backup information of a VM, it consists of two
 *  parts, configuration and list of backups
 *  The schema is as follows:
 *  <BACKUPS>
 *   <BACKUP_CONFIG>
 *     <KEEP_LAST> Just keep the last N backups
 *     <BACKUP_VOLATILE> Backup volatile disks or not
 *     <FS_FREEZE> FS freeze operation to perform on the VM
 *     <LAST_DATASTORE_ID> The dastore ID used to store the active backups(*)
 *     <LAST_BACKUP_ID> ID of the active backup(*)
 *     <LAST_BACKUP_SIZE> SIZE of the active backup(*)
 *   <BACKUP_IDS>
 *     <ID> ID of the image with a valid backup
 *
 *  (*) refers to the active backup operation, and are only present while
 *  a backup is being performed
 *
 *  Configuration attributes defaults
 *    - BACKUP_VOLATILE "NO"
 *    - FS_FREEZE "NONE"
 *    - KEEP_LAST (empty = keep all)
 */
class Backups
{
public:
    Backups();

    ~Backups() = default;

    // *************************************************************************
    // Inititalization functions
    // *************************************************************************

    /**
     *  Builds the snapshot list from its XML representation. This function
     *  is used when importing it from the DB.
     *    @param node xmlNode for the template
     *    @return 0 on success
     */
    int from_xml(const ObjectXML* xml);

    /**
     *  XML Representation of the Snapshots
     */
    std::string& to_xml(std::string& xml) const;

    /**
     *  Gets the BACKUP_CONFIG attribute attribute and parses the associated
     *  attributes:
     *     - BACKUP_VOLATILE
     *     - KEEP_LAST
     *     - FS_FREEZE
     *
     *  The following attributes are stored in the configuration and refers
     *  only to the active backup operation
     *     - LAST_DATASTORE_ID
     *     - LAST_BACKUP_ID
     *     - LAST_BACKUP_SIZE
     */
    int parse(std::string& error_str, Template *tmpl);

    /**
     *  @return true if the backup needs to include volatile disks
     */
    bool do_volatile() const;

    /**
     *  Set of functions to manipulate the LAST_* attributes referring to
     *  the active backup operation
     */
    void last_datastore_id(int ds_id)
    {
        config.replace("LAST_DATASTORE_ID", ds_id);
    }

    void last_backup_id(const std::string& id)
    {
        config.replace("LAST_BACKUP_ID", id);
    }

    void last_backup_size(const std::string& size)
    {
        config.replace("LAST_BACKUP_SIZE", size);
    }

    /* ---------------------------------------------------------------------- */

    int last_datastore_id() const
    {
        int dst;

        config.get("LAST_DATASTORE_ID", dst);

        return dst;
    }

    std::string last_backup_id() const
    {
        std::string id;

        config.get("LAST_BACKUP_ID", id);

        return id;
    }

    std::string last_backup_size() const
    {
        std::string sz;

        config.get("LAST_BACKUP_SIZE", sz);

        return sz;
    }

    /* ---------------------------------------------------------------------- */

    void last_backup_clear()
    {
        config.erase("LAST_DATASTORE_ID");

        config.erase("LAST_BACKUP_ID");
        config.erase("LAST_BACKUP_SIZE");
    }

    /**
     *  @param riids Return the backups that needs to be removed to conform
     *  to KEEP_LAST configuration
     */
    void remove_last(std::set<int> &riids) const
    {
        int kl;

        riids.clear();

        if (!config.get("KEEP_LAST", kl) || kl == 0)
        {
            return;
        }

        auto iids = ids.get_collection();
        auto it   = iids.cbegin();

        int to_remove = iids.size() - kl;

        for (int i = 0 ; i < to_remove && it != iids.cend() ; ++i, ++it)
        {
            riids.insert(*it);
        }
    }

    /**
     *  Adds / deletes a backup from the list. Each backup is represented by
     *  an image in the backup datastore. The list holds the ID's of the images
     *
     *  @return 0 on success -1 if an error adding (already present) or deleting
     *  (not present) occurred
     */
    int add(int id)
    {
        return ids.add(id);
    }

    int del(int id)
    {
        return ids.del(id);
    }

private:
    /**
     *  Text representation of the backup information of the VM
     */
    Template config;

    /**
     *  Backups of the VM as a collection of Image ID
     */
    ObjectCollection ids;
};

#endif /*BACKUPS_H_*/
