#include "User.h"
#include "Character.h"

#include <sigc++/functors/mem_fun.h>
#include <Atlas/Objects/Entity.h>
#include <iostream>

using std::cout;
using std::endl;
using std::cerr;

User::User(const std::string& uid, const std::string& host, short port) :
    m_state(CONNECTING),
    m_uid(uid),
    m_acc(NULL),
    m_charCount(1)
{
    m_con = new Eris::Connection("fuckwad", host, port, false);
    m_con->Connected.connect(sigc::mem_fun(*this, &User::onConnected));
    m_con->Failure.connect(sigc::mem_fun(*this, &User::onConnectionFail));
    m_con->connect();
}

User::~User()
{
    delete m_acc;
    delete m_con;
}

static const char* static_charNames[] = 
{
    "Gabe",
    "Tycho",
    "Batjew",
    "Safety monkey",
    "Div",
    "The Merch",
    "Jesus", // fixme, need description "Is f'ing metal"
    0,
};

void User::createCharacter()
{
    Atlas::Objects::Entity::GameEntity charData;
    
    static int nameCounter = 0;
    
    if (static_charNames[nameCounter] == NULL) nameCounter = 0;
    charData->setName(static_charNames[nameCounter++]);
    
    charData->setParents(std::list<std::string>(1, "settler"));
    
    m_acc->createCharacter(charData);
}

void User::onConnected()
{
    cout << "user connected to server" << endl;
    
    m_acc = new Eris::Account(m_con);
    
    std::string fullName = "Fuckwad User";
    std::string username = m_uid;
    std::string passwd = "secret-romero-love-hideout";
     
    m_acc->createAccount(username, fullName, passwd);
    m_acc->LoginSuccess.connect(sigc::mem_fun(*this, &User::onLoggedIn));
    m_acc->LoginFailure.connect(sigc::mem_fun(*this, &User::onLoginFailure));
    m_acc->AvatarSuccess.connect(sigc::mem_fun(*this, &User::onCharCreated));
    
    m_state = ACC_CREATE;
}

void User::onConnectionFail(const std::string& errMsg)
{
    cerr << "User " << m_uid << " got Eris::Connection failure: " << errMsg << endl;
    m_state = FAILED;
}

void User::onLoggedIn()
{
    m_state = CONNECTED;
    if (m_charCount) createCharacter();
}

void User::onLoginFailure(const std::string& errMsg)
{
    if (m_state == ACC_CREATE) {
        cout << "Failed to create account " << m_uid << ", trying login instead" << endl << std::flush;
        m_acc->login(m_uid, "secret-romero-love-hideout");
        m_state = LOGGING_IN;
    } else {
        m_state = FAILED;
        cerr << "User " << m_uid << " got account login failure: " << errMsg << endl;
    }
}

void User::onCharCreated(Eris::Avatar* av)
{
    Character* c = new Character(av);
    if (--m_charCount) createCharacter();
    m_chars[av->getId()] = c;
}

