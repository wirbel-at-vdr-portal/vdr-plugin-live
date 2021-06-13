/*******************************************************************************
 * users.cpp: User specific rights for the LIVE plugin.
 * See the README file for copyright information and how to reach the author(s).
 ******************************************************************************/
#include <vector>
#include "users.h"
#include "tools.h"
#include "setup.h"
#include <vdr/tools.h>
#include <vdr/config.h>

using vdrlive::MD5Hash;
using vdrlive::StringSplit;
using vdrlive::LiveSetup;


/*******************************************************************************
 * forward declarations
 ******************************************************************************/
class cUser : public cListObject, public iUser {
friend class iUser;
private:
  int m_ID;
  int m_Userrights;
  std::string m_Name;
  std::string m_PasswordMD5;
public:
  cUser(void);
  cUser(int ID, const std::string& Name, const std::string& Password);
  bool Parse(const char* s);
  bool Save(FILE* f);
};


class cUsers : public cConfig<cUser>, public iUsers {
friend class iUsers;
private:
  std::string logged_in_user;
  int UserRights(void);
};



/*******************************************************************************
 * global vars
 ******************************************************************************/
cUsers list;
iUsers& Users = dynamic_cast<iUsers&>(list);
cConfig<cUser>& List = dynamic_cast<cConfig<cUser>&>(list);



/*******************************************************************************
 * class cUser
 * store/restore a single line to/from users.conf 
 ******************************************************************************/
cUser::cUser(void) : m_ID(-1), m_Userrights(0) {}

cUser::cUser(int ID, const std::string& Name, const std::string& Password)
 : m_ID(ID), m_Name(Name) {
  SetPassword(Password);
}

bool cUser::Parse(const char* s) {
  auto parts = StringSplit(s, ':');

  if (parts.size() != 4)
     return false;

  m_ID          = std::stoi(parts[0]);
  m_Name        = parts[1];
  m_PasswordMD5 = parts[2];
  m_Userrights  = std::stoi(parts[3]);

  return true;
}

bool cUser::Save(FILE* f) {
  std::string line;

  line = std::to_string(m_ID) + ':' +
         m_Name + ':' +
         m_PasswordMD5 + ':' +
         std::to_string(m_Userrights);

  return fprintf(f, "%s\n", line.c_str()) > 0;
}


/*******************************************************************************
 * class cUsers
 ******************************************************************************/

int cUsers::UserRights(void) {
  if (!LiveSetup().UseAuth() or logged_in_user == LiveSetup().GetAdminLogin())
     return UR_ADMIN;

  iUser* u = list.GetByName(logged_in_user);
  return u ? u->Userrights() : 0;
}


/*******************************************************************************
 * class iUser
 * the interface to all the plugin needs to know about a single user.
 ******************************************************************************/
iUser::~iUser() {}

int iUser::Id(void) {
  return dynamic_cast<cUser*>(this)->m_ID;
}

std::string iUser::Name(void) {
  return dynamic_cast<cUser*>(this)->m_Name;
}

std::string iUser::PasswordMD5(void) {
  return dynamic_cast<cUser*>(this)->m_PasswordMD5;
}

int iUser::Userrights(void) {
  return dynamic_cast<cUser*>(this)->m_Userrights;
}

int iUser::GetPasswordLength(void) {
  // format is <length>|<md5-hash of password>
  auto parts = StringSplit( PasswordMD5(), '|' );
  return (parts.size() > 0) ? std::stoi(parts[0]) : 0;
}

std::string iUser::GetMD5HashPassword(void) {
  // format is <length>|<md5-hash of password>
  auto parts = StringSplit( PasswordMD5(), '|' );
  return (parts.size() > 1) ? parts[1] : "";
}

void iUser::SetId(int Id) {
  dynamic_cast<cUser*>(this)->m_ID = Id;
}

void iUser::SetName(const std::string& Name) {
  dynamic_cast<cUser*>(this)->m_Name = Name;
}

void iUser::SetPassword(const std::string& Password) {
  std::string s = std::to_string((int)Password.size()) + "|" + MD5Hash(Password);
  dynamic_cast<cUser*>(this)->m_PasswordMD5 = s;
}

void iUser::SetUserrights(int Userrights) {
  dynamic_cast<cUser*>(this)->m_Userrights = Userrights;
}



/*******************************************************************************
 * class iUsers
 * the interface to the list of users.
 ******************************************************************************/
iUsers::~iUsers() {}

iUser* iUsers::Add(std::string Name, std::string Password) {
  int ID = -1;

  for(cUser* u = List.First(); u; u = List.Next(u)) {
     if (u->Id() > ID)
        ID = u->Id();
     }

  cUser* u = new cUser(++ID, Name, Password);
  List.Add(u);
  return dynamic_cast<iUser*>(u);
}

void iUsers::Del(std::string Id) {
  iUser* u = GetById(Id);

  if (u) List.Del(dynamic_cast<cUser*>(u));
}

iUser* iUsers::First(void) {
  return dynamic_cast<iUser*>(List.First());
}

iUser* iUsers::Next(iUser* user) {
  cUser* u = dynamic_cast<cUser*>(user);
  u = List.Next(u);
  return dynamic_cast<iUser*>(u);
}

bool iUsers::Load(const char* FileName) {
  return List.Load(FileName, true);
}

bool iUsers::Save(void) {
  return List.Save();
}

void iUsers::SetCurrentUser(std::string Name) {
  list.logged_in_user = Name;
}

bool iUsers::ValidLogin(const std::string& login, const std::string& password) {
  if (login == LiveSetup().GetAdminLogin() && MD5Hash(password) == LiveSetup().GetMD5HashAdminPassword())
     return true;

  iUser* user = GetByName(login);
  return (user && user->GetMD5HashPassword() == MD5Hash(password));
}

iUser* iUsers::GetById(const std::string& Id) {
  int id = std::stoi(Id);
  for(cUser* u = List.First(); u; u = List.Next(u)) {
     if (u->Id() == id)
        return dynamic_cast<iUser*>(u);
     }
  return nullptr;
}

iUser* iUsers::GetByName(const std::string& Name) {
  for(cUser* u = List.First(); u; u = List.Next(u)) {
     if (u->Name() == Name)
        return dynamic_cast<iUser*>(u);
     }
  return nullptr;
}

bool iUsers::UserMayEditSetup(void)          { return list.UserRights() & UR_EDITSETUP;   }
bool iUsers::UserMayEditTimers(void)         { return list.UserRights() & UR_EDITTIMERS;  }
bool iUsers::UserMayDeleteTimers(void)       { return list.UserRights() & UR_DELTIMERS;   }
bool iUsers::UserMayDeleteRecs(void)         { return list.UserRights() & UR_DELRECS;     }
bool iUsers::UserMayUseRemote(void)          { return list.UserRights() & UR_USEREMOTE;   }
bool iUsers::UserMayStartReplay(void)        { return list.UserRights() & UR_STARTREPLAY; }
bool iUsers::UserMaySwitchChannel(void)      { return list.UserRights() & UR_SWITCHCHNL;  }
bool iUsers::UserMayEditSearchTimers(void)   { return list.UserRights() & UR_EDITSTIMERS; }
bool iUsers::UserMayDeleteSearchTimers(void) { return list.UserRights() & UR_DELSTIMERS;  }
bool iUsers::UserMayEditRecs(void)           { return list.UserRights() & UR_EDITRECS;    }
