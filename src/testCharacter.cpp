// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001-2003 Alistair Riddoch and James Turner

#include "ClientConnection.h"
#include "testCharacter.h"
#include "process_debug.h"

#include <iostream>

using Atlas::Message::Element;
using Atlas::Message::MapType;
using Atlas::Message::ListType;

void testInvalidCharacterCreate(ClientConnection &c)
{
    verbose( std::cout << "Testing character creation with invalid type"
                           << std::endl << std::flush; );

    MapType character;
    character["objtype"] = "object";
    character["parents"] = ListType(1,"__bad__type__");
    character["name"] = "Dwayne";

    Create create;
    create->setFrom(c.getAccountId());
    create->setArgsAsList(ListType(1,character));
    
    int sno = c.send(create);

    verbose( std::cout << "Waiting for error response to invalid character create"
                           << std::endl << std::flush; );

    if (c.waitForError(sno)) {
        std::cerr << "ERROR: Invalid Character creation did not result in error"
                  << std::endl << std::flush;
    }
}

void testCharacterCreate(ClientConnection & connection1,
                         ClientConnection & connection2,
                         ClientConnection & connection3)
{
    verbose( std::cout << "Creating character on primary connection"
                       << std::endl << std::flush; );

    MapType character;
    character["objtype"] = "object";
    character["parents"] = ListType(1,"settler");
    character["name"] = "Nivek";

    connection1.createChar(character);
    verbose(std::cout << "created character with ID " << connection1.getCharacterId() <<
            " on connection 1" << std::endl << std::flush; );
    
    if (connection2.isOpen()) {
        character["name"] = "Civen";
        connection2.createChar(character);
        verbose(std::cout << "created character with ID " << connection2.getCharacterId() <<
            " on connection 2" << std::endl << std::flush; );
    }

    if (connection3.isOpen()) {
        testInvalidCharacterCreate(connection3);
        
        verbose( std::cout << "Creating character on third connection"
                           << std::endl << std::flush; );

        character["name"] = "Dwayne";
        connection3.createChar(character);
        verbose(std::cout << "created character with ID " << connection3.getCharacterId() <<
            " on connection 3" << std::endl << std::flush; );
    }

    verbose( std::cout << "Modifying appearance of character on primary connection"
                       << std::endl << std::flush; );

    MapType charMod;
    charMod["id"] = connection1.getCharacterId();
    charMod["height"] = 4;
    MapType gmap;
    gmap["foo"] = "bar";
    charMod["guise"] = gmap;
    Atlas::Objects::Operation::Set set;
    set->setFrom(connection1.getAccountId());
    set->setArgsAsList(ListType(1,charMod));

    connection1.send(set);
}
