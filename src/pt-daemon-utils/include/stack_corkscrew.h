////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __STACK_CORKSCREW_H__
#define __STACK_CORKSCREW_H__

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "common.h"

#include "json_utils.h"

#include <stdint.h>

#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

#define pid_t uint32_t

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef uint32_t StackToken;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class StackFrame
{
public:

  StackFrame ()
    : m_level (0)
    , m_pc (0)
    , m_sp (0)
    , m_functionToken (0)
  {
    m_function [0] = '\0';
  }

  size_t m_level;

  uint64_t m_pc;

  uint64_t m_sp;

  char m_function [128];

  StackToken m_functionToken;

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class StackCorkscrew
{
public:

  StackCorkscrew ();

  ~StackCorkscrew ();

  size_t Unwind (pid_t ppid, pid_t tid, size_t ignoreDepth, size_t maxDepth);

  size_t LoadFromJson (JsonNode &node);

  bool ConvertToJson (std::string &output) const;

  bool GetFrame (size_t index, StackFrame &frame) const;

  size_t GetNumFrames () const;

private:

  friend class StackSymbolicator;

  std::vector <StackFrame> m_frames;

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // __STACK_CORKSCREW_H__

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
