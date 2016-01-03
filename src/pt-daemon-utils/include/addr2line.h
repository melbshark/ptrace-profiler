////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __ADDR2LINE_H__
#define __ADDR2LINE_H__

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>

#include <unordered_map>

#include <vector>

#ifdef WIN32
#include <windows.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
  StdOutRead,
  StdOutWrite,
  StdErrRead,
  StdErrWrite,
  StdInRead,
  StdInWrite,
  NumPipeTypes
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



struct Addr2LineProcess
{
#ifdef WIN32

  HANDLE pipes [NumPipeTypes];

  PROCESS_INFORMATION processInfo;

#endif

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Addr2Line
{
public:

  Addr2Line (const std::string &addr2lineTool, const std::vector <std::string> *sysroots);

  ~Addr2Line ();

  bool Symbolicate (const std::string &lib, const std::string &address, std::string *symbol);

private:

  bool CreateChildProcess (const std::string &lib, Addr2LineProcess *process);

  bool DestroyChildProcess (const std::string &lib, Addr2LineProcess *process);

  bool WriteToChildProcess (Addr2LineProcess *process, char *buffer, size_t bytesToWrite);

  bool ReadFromChildProcess (Addr2LineProcess *process, std::string *stdoutBuffer, std::string *stderrBuffer);

  std::string m_addr2lineTool;

  const std::vector <std::string> *m_sysroots;

  std::unordered_map <std::string, Addr2LineProcess> m_instances;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // __ADDR2LINE_H__

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////