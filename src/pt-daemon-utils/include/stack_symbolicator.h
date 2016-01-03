////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __STACK_SYMBOLICATOR_H__
#define __STACK_SYMBOLICATOR_H__

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "common.h"

#include "addr2line.h"

#include "mmap_manager.h"

#include "stack_corkscrew.h"

#include <unordered_map>

#include <string>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class StackSymbolicator
{
public:

  StackSymbolicator (MemoryMapManager *mmapManager, Addr2Line *add2line);

  void Preprocess (StackCorkscrew *corkscrew);

  void Symbolicate ();

  const std::unordered_map <uint64_t, StackToken> GetTokensByPcMap () const { return m_tokensByPc; }

  const std::unordered_map <uint32_t, std::string> GetLocationsByTokenMap () const { return m_locationsByToken; }

private:

  MemoryMapManager *m_mmapManager;

  Addr2Line *m_addr2line;

  uint32_t m_currentToken;

  std::unordered_map <uint64_t, StackToken> m_tokensByPc;

  std::unordered_map <uint32_t, std::string> m_locationsByToken;

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // __STACK_SYMBOLICATOR_H__

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////