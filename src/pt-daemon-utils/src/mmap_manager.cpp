////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "mmap_manager.h"

#include <cassert>

#include <errno.h>

#include <string>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MemoryMapManager::MemoryMapManager ()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MemoryMapManager::~MemoryMapManager ()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

size_t MemoryMapManager::ParseJsonNode (const JsonNode &mapsRoot)
{
  // 
  // Parse a JSON array containing map region data. Format described in ConvertToJSON.
  // 

  assert (mapsRoot.IsArray ());

  m_regions.clear ();

  const size_t arrayLen = mapsRoot.GetLength ();

  for (size_t i = 0; i < arrayLen; ++i)
  {
    JsonNode node;

    if (!mapsRoot.TryChildNodeAtIndex (i, &node) || !node.IsValid ())
    {
      continue;
    }

    MemoryMapRegion map;

    memset (&map, 0, sizeof (map));

    char buffer [64] = "";

    if (!JsonUtils::TryChildAsString (node, "start", buffer, sizeof (buffer)))
    {
      continue;
    }

    map.start = strtoll (buffer, NULL, 16);

    if (!JsonUtils::TryChildAsString (node, "end", buffer, sizeof (buffer)))
    {
      continue;
    }

    map.end = strtoll (buffer, NULL, 16);

    if (!JsonUtils::TryChildAsString (node, "name", map.pathname, sizeof (map.pathname)))
    {
      continue;
    }

    if (JsonUtils::TryChildAsString (node, "offset", buffer, sizeof (buffer)))
    {
      map.offset = strtoll (buffer, NULL, 16);
    }

    m_regions.push_back (map);
  }

  return m_regions.size ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

size_t MemoryMapManager::ParseUnixProcessMapsFile (const char *filename)
{
  assert (filename != NULL);

  m_regions.clear ();

  FILE *mapsFile = fopen (filename, "r");

  if (!mapsFile)
  {
    fprintf (stderr, "Failed to open process maps file (%s). %s.\n", filename, strerror (errno));

    fflush (stderr);

    errno = 0;

    return 0;
  }

  int result = 0;

  while (true)
  {
    MemoryMapRegion mapData;

    result = fscanf (mapsFile, "%lx%lx %4s %lx %*x:%*x %ld", &mapData.start, &mapData.end, mapData.permissions, &mapData.offset, &mapData.inode);

    if ((result <= 0) || (result == EOF))
    {
      break;
    }

    // 
    // The length and/or presence of a map region name is rather inconsistent, so we need to parse carefully.
    // 

    char buffer [1024];

    size_t i = 0;

    do
    {
      fread (&buffer [i], 1, 1, mapsFile);
    }
    while (buffer [i++] != '\n');

    buffer [i - 1] = '\0';

    const char *pathname = buffer;

    while (*pathname == ' ')
    {
      pathname += 1;
    }

    size_t pathnameLen = strlen (pathname);

    const bool validEntry = (pathnameLen >= 7) // at very least must be libX.so
      && ((strncmp (pathname + (pathnameLen -  3), ".so", 3) == 0)
      || (strncmp (pathname + (pathnameLen - 4), ".dex", 4) == 0)
      || (strncmp (pathname + (pathnameLen - 4), ".oat", 4) == 0));

    if (validEntry)
    {
      strcpy (mapData.pathname, pathname);

      m_regions.push_back (mapData);
    }

    result = feof (mapsFile);

    if ((result != 0) || (result == EOF))
    {
      break;
    }
  }

  fclose (mapsFile);

  return m_regions.size ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const MemoryMapRegion *MemoryMapManager::FindMapForAddress (uint64_t address) const
{
  for (size_t i = 0; i < m_regions.size (); ++i)
  {
    const MemoryMapRegion *region = &m_regions [i];

    if (address > region->end)
    {
      continue;
    }
    else if (address < region->start)
    {
      continue;
    }
    else if (address >= region->start)
    {
      return region;
    }
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MemoryMapManager::ConvertToJSON (std::string &json)
{
  std::vector <MemoryMapRegion>::const_iterator it = m_regions.begin ();

  json.clear ();

  json.append ("\"maps\":[");

  char buffer [256];

  bool commaSeperated = false;

  while (it != m_regions.end ())
  {
    const MemoryMapRegion &map = *it;

    sprintf (buffer, "%s{\"start\":\"0x%llx\",\"end\":\"0x%llx\",\"offset\":\"0x%llx\",\"name\":\"%s\"}", (commaSeperated ? "," : ""), map.start, map.end, map.offset, map.pathname);

    json.append (buffer);

    commaSeperated = true;

    it++;
  }

  json.append ("]");

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
