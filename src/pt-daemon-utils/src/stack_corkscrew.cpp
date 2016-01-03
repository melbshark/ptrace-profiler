////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "common.h"

#include "stack_corkscrew.h"

#include <algorithm>

#include <cassert>

#include <cstring>

#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define UNWIND_STACK_POINTER 1

#define UNWIND_FUNCTION_NAME 0

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

StackCorkscrew::StackCorkscrew ()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

StackCorkscrew::~StackCorkscrew ()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

size_t StackCorkscrew::Unwind (pid_t ppid, pid_t tid, size_t ignoreDepth, size_t maxDepth)
{
  assert (ppid > 0);

  assert (tid >= ppid);

  m_frames.clear ();

  if (maxDepth == 0)
  {
    return 0;
  }

#ifdef UNW_VERSION

  static unw_addr_space_t addr_space = unw_create_addr_space (&_UPT_accessors, 0);

  if (!addr_space)
  {
    fprintf (stderr, "unw_create_addr_space failed.\n");

    fflush (stderr);

    return 0;
  }

#if UNWIND_FUNCTION_NAME

  unw_map_cursor_t map_cursor;

  if (unw_map_cursor_create (&map_cursor, tid) < 0)
  {
    fprintf (stderr, "Failed to create map cursor.\n");

    fflush (stderr);

    return 0;
  }

  unw_map_set (addr_space, &map_cursor);

#endif

  struct UPT_info* upt_info = reinterpret_cast<struct UPT_info*> (_UPT_create (tid));

  if (!upt_info)
  {
    fprintf (stderr, "Failed to create upt info.\n");

    fflush (stderr);

    return 0;
  }

  unw_cursor_t cursor;

  {
    int error = unw_init_remote (&cursor, addr_space, upt_info);

    if (error < 0)
    {
      fprintf (stderr, "unw_init_remote failed (%d)\n", error);

      fflush (stderr);

      return 0;
    }
  }

#endif

  bool shouldContinue = false;

  size_t numFrames = 0;

  do
  {
    // 
    // Evaluate instruction pointer / program counter address.
    // 

    uint64_t pc = 0;

#ifdef UNW_VERSION

    {
      unw_word_t unwound_pc;

      int error = unw_get_reg (&cursor, UNW_REG_IP, &unwound_pc);

      if (error < 0)
      {
        fprintf (stderr, "Failed to read IP (%d)\n", error);

        fflush (stderr);

        break;
      }

      pc = unwound_pc;
    }

#endif

    uint64_t sp = 0;

#ifdef UNW_VERSION

  #if UNWIND_STACK_POINTER

    {
      unw_word_t unwound_sp;

      int error = unw_get_reg (&cursor, UNW_REG_SP, &unwound_sp);

      if (error < 0)
      {
        fprintf (stderr, "Failed to read SP (%d)\n", error);

        fflush (stderr);

        break;
      }

      sp = unwound_sp;
    }

  #endif

#endif

    if (ignoreDepth == 0)
    {
      const char *function = "";

    #if UNWIND_FUNCTION_NAME

      uintptr_t offset = 0;

      char buffer [128];

      unw_word_t value;

      function = "??";

      const int result = unw_get_proc_name_by_ip (addr_space, pc, buffer, sizeof (buffer), &value, upt_info);

      if (result >= 0 && buffer [0] != '\0')
      {
        function = buffer;

        offset = static_cast<uintptr_t>(value);
      }

    #endif

      StackFrame frame;

      frame.m_level = numFrames;

      frame.m_pc = pc;

    #if UNWIND_STACK_POINTER

      frame.m_sp = sp;

    #endif

      strncpy (frame.m_function, function, sizeof (frame.m_function));

      m_frames.push_back (frame);

      numFrames++;
    }
    else
    {
      ignoreDepth--;
    }

  #ifdef UNW_VERSION

    shouldContinue = (unw_step (&cursor) > 0);

  #endif
  }
  while (shouldContinue && numFrames < maxDepth);

#ifdef UNW_VERSION

  _UPT_destroy (upt_info);

  #if UNWIND_FUNCTION_NAME

  unw_map_cursor_destroy (&map_cursor);

  unw_map_cursor_clear (&map_cursor);

  #endif

#endif

  return m_frames.size ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

size_t StackCorkscrew::LoadFromJson (JsonNode &node)
{
  assert (node.IsArray ());

  for (size_t i = 0; i < node.GetLength (); ++i)
  {
    JsonNode frameNode;

    if (!node.TryChildNodeAtIndex (i, &frameNode))
    {
      break;
    }

    StackFrame frame;

    JsonNode levelNode, pcNode, spNode, funcNode;

    if (frameNode.TryChild ("level", &levelNode) && (levelNode.IsInteger ()))
    {
      int32_t level = frame.m_level;

      levelNode.GetInteger (level);

      frame.m_level = (size_t) level;
    }

    char buffer [32] = "";

    if (frameNode.TryChild ("pc", &pcNode) && (pcNode.IsString ()))
    {
      buffer [0] = '\0';

      pcNode.GetString (buffer, sizeof (buffer));

      frame.m_pc = strtoll (buffer, NULL, 16);
    }

    if (frameNode.TryChild ("sp", &spNode) && (spNode.IsString ()))
    {
      buffer [0] = '\0';

      spNode.GetString (buffer, sizeof (buffer));

      frame.m_sp = strtoll (buffer, NULL, 16);
    }

    if (frameNode.TryChild ("func", &funcNode))
    {
      frame.m_function [0] = '\0';

      if (funcNode.IsString ())
      {
        funcNode.GetString (frame.m_function, sizeof (frame.m_function));
      }
      else if (funcNode.IsInteger ())
      {
        int32_t token;

        funcNode.GetInteger (token);

        frame.m_functionToken = token;
      }
    }

    m_frames.push_back (frame);
  }

  return m_frames.size ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool StackCorkscrew::ConvertToJson (std::string &output) const
{
  output.append ("\"frames\":[");

  char workingBuffer [256];

  for (size_t i = 0; i < m_frames.size (); ++i)
  {
    const StackFrame &frame = m_frames [i];

    snprintf (workingBuffer, sizeof (workingBuffer), "%s{\"level\":%d,\"pc\":\"%llx\",\"sp\":\"%llx\",\"func\":\"%s\"}", ((i > 0) ? "," : ""), frame.m_level, frame.m_pc, frame.m_sp, frame.m_function);

    output.append (workingBuffer);
  }

  output.append ("]");

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool StackCorkscrew::GetFrame (size_t index, StackFrame &frame) const
{
  if (index >= m_frames.size ())
  {
    return false;
  }

  frame = m_frames [index];

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

size_t StackCorkscrew::GetNumFrames () const
{
  return m_frames.size ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
