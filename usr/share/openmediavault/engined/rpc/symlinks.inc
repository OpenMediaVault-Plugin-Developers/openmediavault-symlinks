<?php
/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @copyright Copyright (c) 2013-2017 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
require_once("openmediavault/globals.inc");
require_once("openmediavault/functions.inc");

class OMVRpcServiceSymlinks extends \OMV\Rpc\ServiceAbstract
{
    public function getName()
    {
        return "Symlinks";
    }

    public function initialize()
    {
        $this->registerMethod("getSymlinkList");
        $this->registerMethod("getSymlink");
        $this->registerMethod("setSymlink");
        $this->registerMethod("deleteSymlink");
    }

    public function getSymlinkList($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.getlist");
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        $objects = $db->getAssoc("conf.service.symlinks.symlink");
        // Filter the result.
        return $this->applyFilter($objects, $params['start'], $params['limit'],
            $params['sortfield'], $params['sortdir']);
    }

    public function getSymlink($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.objectuuid");
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        return $db->getAssoc("conf.service.symlinks.symlink", $params['uuid']);
    }

    public function setSymlink($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.symlinks.setsymlink");
        // Prepare the configuration object.
        $object = new \OMV\Config\ConfigObject("conf.service.symlinks.symlink");
        $object->setAssoc($params);
        // Set the configuration object.
        $isNew = $object->isNew();
        $db = \OMV\Config\Database::getInstance();
        // remove old symbolic link if modifying existing symlink
        if ($isNew === false) {
            $oldObject = $db->getAssoc("conf.service.symlinks.symlink", $params['uuid']);
            if (is_link($oldObject['destination'])) {
                unlink($oldObject['destination']);
            }
        }
        $db->set($object);

        if (($object->get('enable') === true) &&
            (is_link($object->get('destination')) !== true)) {
            // Create symbolic link
            $success = false;
            $success = symlink($object->get('source'), $object->get('destination'));
            if (false === $success) {
                throw new \OMV\Exception(
                    sprintf(
                        "Failed to create symbolic link at %s",
                        $object->get('destination')
                    )
                );
            }
        }

        // Return the configuration object.
        return $object->getAssoc();
    }

    public function deleteSymlink($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.objectuuid");
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.symlinks.symlink", $params['uuid']);
        // remove old symbolic link
        if (is_link($object->get('destination'))) {
            unlink($object->get('destination'));
        }
        // Delete the configuration object.
        $db->delete($object);
        // Return the deleted configuration object.
        return $object->getAssoc();
    }
}
