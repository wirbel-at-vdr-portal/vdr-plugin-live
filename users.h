/*******************************************************************************
 * users.h: User specific rights for the LIVE plugin.
 * See the README file for copyright information and how to reach the author(s).
 ******************************************************************************/
#pragma once

/* do not include any VDR, cxxtools or tntnet headers here. */
#include <string>


enum eUserRights {
  UR_EDITSETUP   = (1 << 0),
  UR_EDITTIMERS  = (1 << 1),
  UR_DELTIMERS   = (1 << 2),
  UR_DELRECS     = (1 << 3),
  UR_USEREMOTE   = (1 << 4),
  UR_STARTREPLAY = (1 << 5),
  UR_SWITCHCHNL  = (1 << 6),
  UR_EDITSTIMERS = (1 << 7),
  UR_DELSTIMERS  = (1 << 8),
  UR_EDITRECS    = (1 << 9),
  UR_ADMIN       = ~0
};

class iUser {
public:
  virtual ~iUser();
  int Id(void);
  std::string Name(void);
  std::string PasswordMD5(void);
  int Userrights(void);
  int GetPasswordLength(void);
  std::string GetMD5HashPassword(void);
  void SetId(int Id);
  void SetName(const std::string& Name);
  void SetPassword(const std::string& Password);
  void SetUserrights(int Userrights);
};

class iUsers {
public:
  virtual ~iUsers();
  bool ValidLogin(const std::string& login, const std::string& password);
  void SetCurrentUser(std::string Name);
  std::string CurrentUser(void);

  bool UserMayEditSetup(void);
  bool UserMayEditTimers(void);
  bool UserMayDeleteTimers(void);
  bool UserMayDeleteRecs(void);
  bool UserMayUseRemote(void);
  bool UserMayStartReplay(void);
  bool UserMaySwitchChannel(void);
  bool UserMayEditSearchTimers(void);
  bool UserMayDeleteSearchTimers(void);
  bool UserMayEditRecs(void);

  iUser* First(void);
  iUser* Next(iUser* user);
  iUser* GetById(const std::string& Id);
  iUser* GetByName(const std::string& Name);
  iUser* Add(std::string Name, std::string Password);
  void Del(std::string Id);
  bool Load(const char* FileName);
  bool Save(void);
};

extern iUsers& Users;
