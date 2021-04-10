#include "pages.h"
#include "globals.h"
#include "utilities.h"

// This namespace maps from virtual page "addresses" to addresses/bytes of real in memory entities
//
// For TunerStudio:
// 1. Each page has a numeric identifier (0 to N-1)
// 2. A single page is a continguous block of data.
// So individual bytes are identified by a page number + offset
//
// The TS layout is not what is in memory. E.g.
//
//    TS Page 2               |0123456789ABCD|0123456789ABCDEF|
//                                |                       |
//    Arduino In Memory  |--- Entity A ---|         |--- Entity B -----|
//
// Further, the in memory entity may also not be contiguous or in the same
// order that TS expects
//
// So there is a 2 stage mapping:
//  1. Page # + Offset to entity
//  2. Offset to intra-entity byte

// Page sizes as defined in the .ini file
static constexpr const uint16_t ini_page_sizes[] = { 0, 128, 288, 288, 128, 288, 128, 240, 384, 192, 192, 288, 192, 128, 288 };

// ==================================== Offset to Entity Mapping =========================

struct entity_t {
  // The entity that the offset mapped to
  void *pEntity;
  uint16_t table_key;
  uint16_t start; // The start position of the entity, in bytes, from the start of the page
  uint16_t size;  // Size of the entity in bytes
  entity_type type;
};

// This will fail AND print the page number and required size
template <uint8_t pageNum, uint16_t min>
static inline void check_size() {
  static_assert(ini_page_sizes[pageNum] >= min, "Size is off!");
}

// Handy table macros
#define TABLE_VALUE_END(size) ((uint16_t)size*(uint16_t)size)
#define TABLE_AXISX_END(size) (TABLE_VALUE_END(size)+(uint16_t)size)
#define TABLE_AXISY_END(size) (TABLE_AXISX_END(size)+(uint16_t)size)
#define TABLE_SIZE(size) TABLE_AXISY_END(size)

// Precompute for performance
#define TABLE16_SIZE TABLE_SIZE(16)
#define TABLE8_SIZE TABLE_SIZE(8)
#define TABLE6_SIZE TABLE_SIZE(6)
#define TABLE4_SIZE TABLE_SIZE(4)

/**
 * This is required to reduce RAM (.data section) usage. 
 * 
 * Without this method as an intermediary, the compiler will determine
 * that ALL of the entity_t instances are compile time constants. So
 * it will place the constants in the .data section. We have ~76 
 * entity_t instances. So these constants would consume >675 bytes.
 * 
 * If the entity_t instances are compile time constants, then all the 
 * parameters to this function are also compile time constants. But since
 * there are distinct parameter combinations, the compiler is much more
 * space efficient. 
 */
static inline entity_t create_entity(void *pEntity, uint16_t table_key,
                                      uint16_t start, uint16_t size, entity_type type)
{
  return { pEntity, table_key, .start = start, .size = size, .type = type };
}

// Signal the end of a page
#define END_OF_PAGE(pageNum, pageSize) \
  check_size<pageNum, pageSize>(); \
  return create_entity(nullptr, 0, 0, pageSize, End);

// If the offset is in range, create a None entity_t
#define CHECK_NOENTITY(offset, startByte, blockSize) \
  if (offset < (startByte)+blockSize) \
  { \
    return create_entity(nullptr, 0, startByte, blockSize, NoEntity); \
  } 

// If the offset is in range, create a Table entity_t
#define CHECK_TABLE(offset, startByte, pTable, tableSize) \
  if (offset < (startByte)+TABLE_SIZE(tableSize)) \
  { \
    return create_entity(pTable, (pTable)->type_key, startByte, TABLE_SIZE(tableSize), Table); \
  }

// If the offset is in range, create a Raw entity_t
#define CHECK_RAW(offset, startByte, pDataBlock, blockSize) \
  if (offset < (startByte)+blockSize) \
  { \
    return create_entity(pDataBlock, 0U, startByte, blockSize, Raw); \
  } 

// Does the heavy lifting of mapping page+offset to an entity
//
// Alternative implementation would be to encode the mapping into data structures
// That uses flash memory, which is scarce. And it was too slow.
static inline __attribute__((always_inline)) // <-- this is critical for performance
entity_t map_page_offset_to_entity_inline(uint8_t pageNumber, uint16_t offset)
{
  switch (pageNumber)
  {
    case 0:
      END_OF_PAGE(0, 0);

    case veMapPage:
      CHECK_TABLE(offset, 0U, &fuelTable, 16)
      END_OF_PAGE(veMapPage, TABLE16_SIZE);

    case ignMapPage: //Ignition settings page (Page 2)
      CHECK_TABLE(offset, 0U, &ignitionTable, 16)
      END_OF_PAGE(ignMapPage, TABLE16_SIZE);

    case afrMapPage: //Air/Fuel ratio target settings page
      CHECK_TABLE(offset, 0U, &afrTable, 16)
      END_OF_PAGE(afrMapPage, TABLE16_SIZE);

    case boostvvtPage: //Boost, VVT and staging maps (all 8x8)
      CHECK_TABLE(offset, 0U, &boostTable, 8)
      CHECK_TABLE(offset, TABLE8_SIZE, &vvtTable, 8)
      CHECK_TABLE(offset, TABLE8_SIZE*2, &stagingTable, 8)
      END_OF_PAGE(boostvvtPage, TABLE8_SIZE*3);

    case seqFuelPage:
      CHECK_TABLE(offset, 0U, &trim1Table, 6)
      CHECK_TABLE(offset, TABLE6_SIZE*1, &trim2Table, 6)
      CHECK_TABLE(offset, TABLE6_SIZE*2, &trim3Table, 6)
      CHECK_TABLE(offset, TABLE6_SIZE*3, &trim4Table, 6)
      CHECK_TABLE(offset, TABLE6_SIZE*4, &trim5Table, 6)
      CHECK_TABLE(offset, TABLE6_SIZE*5, &trim6Table, 6)
      CHECK_TABLE(offset, TABLE6_SIZE*6, &trim7Table, 6)
      CHECK_TABLE(offset, TABLE6_SIZE*7, &trim8Table, 6)
      END_OF_PAGE(seqFuelPage, TABLE6_SIZE*8);

    case fuelMap2Page:
      CHECK_TABLE(offset, 0U, &fuelTable2, 16)
      END_OF_PAGE(fuelMap2Page, TABLE16_SIZE);

    case wmiMapPage:
      CHECK_TABLE(offset, 0U, &wmiTable, 8)
      CHECK_TABLE(offset, TABLE8_SIZE, &vvt2Table, 8)
      CHECK_TABLE(offset, TABLE8_SIZE*2, &dwellTable, 4)
      END_OF_PAGE(wmiMapPage, TABLE8_SIZE*2 + TABLE4_SIZE);
      break;
    
    case ignMap2Page:
      CHECK_TABLE(offset, 0U, &ignitionTable2, 16)
      END_OF_PAGE(ignMap2Page, TABLE16_SIZE);

    case veSetPage: 
      CHECK_RAW(offset, 0U, &configPage2, sizeof(configPage2))
      END_OF_PAGE(veSetPage, sizeof(configPage2));

    case ignSetPage: 
      CHECK_RAW(offset, 0U, &configPage4, sizeof(configPage4))
      END_OF_PAGE(ignSetPage, sizeof(configPage4));

    case afrSetPage: 
      CHECK_RAW(offset, 0U, &configPage6, sizeof(configPage6))
      END_OF_PAGE(afrSetPage, sizeof(configPage6));

    case canbusPage:  
      CHECK_RAW(offset, 0U, &configPage9, sizeof(configPage9))
      END_OF_PAGE(canbusPage, sizeof(configPage9));

    case warmupPage: 
      CHECK_RAW(offset, 0U, &configPage10, sizeof(configPage10))
      END_OF_PAGE(warmupPage, sizeof(configPage10));

    case progOutsPage: 
      CHECK_RAW(offset, 0U, &configPage13, sizeof(configPage13))
      END_OF_PAGE(progOutsPage, sizeof(configPage13));

    default:
      abort(); // Unkown page number. Not a lot  we can do.
      break;
  }
}

// =============================== Table function calls =========================

// With no templates or inheritance we need some way to call functions
// for the various distinct table types. CONCRETE_TABLE_ACTION dispatches
// to a caller defined function overloaded by the type of the table. 
#define CONCRETE_TABLE_ACTION_INNER(size, xDomain, yDomain, action, ...) \
  case DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)::type_key: action(size, xDomain, yDomain, ##__VA_ARGS__);
#define CONCRETE_TABLE_ACTION(testKey, action, ...) \
  switch (testKey) { \
  TABLE_GENERATOR(CONCRETE_TABLE_ACTION_INNER, action, ##__VA_ARGS__ ) \
  default: abort(); }

// ================================== Table getters & setters ===================

#define OFFSET_TO_XAXIS_INDEX(offset, size) (offset - sq(size))

#define GET_XAXIS_VALUE(size, xDomain, yDomain, pTable, offset) \
   return ((DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)*)pTable)->axisX[OFFSET_TO_XAXIS_INDEX(offset, size)] / getTableAxisFactor(axis_domain_ ## xDomain);

static inline uint8_t get_xaxis(uint16_t table_key, void* pTable, uint16_t offset)
{
  CONCRETE_TABLE_ACTION(table_key, GET_XAXIS_VALUE, pTable, offset)
}

#define SET_XAXIS_VALUE(size, xDomain, yDomain, pTable, offset, value) \
    ((DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)*)pTable)->axisX[OFFSET_TO_XAXIS_INDEX(offset, size)] = value * getTableAxisFactor(axis_domain_ ## xDomain); \
    break;

static inline void set_xaxis(uint16_t table_key, void* pTable, uint16_t offset, byte value)
{
  CONCRETE_TABLE_ACTION(table_key, SET_XAXIS_VALUE, pTable, offset, value)
}

#define OFFSET_TO_YAXIS_INDEX(offset, size) ((size-1) - (offset - (sq(size)+size)))

#define GET_YAXIS_VALUE(size, xDomain, yDomain, pTable, offset) \
    return ((DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)*)pTable)->axisY[OFFSET_TO_YAXIS_INDEX(offset, size)] / getTableAxisFactor(axis_domain_ ## yDomain);

static inline uint8_t get_yaxis(uint16_t table_key, void* pTable, uint16_t offset)
{
  CONCRETE_TABLE_ACTION(table_key, GET_YAXIS_VALUE, pTable, offset)
}

#define SET_YAXIS_VALUE(size, xDomain, yDomain, pTable, offset, value) \
    ((DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)*)pTable)->axisY[OFFSET_TO_YAXIS_INDEX(offset, size)] = value * getTableAxisFactor(axis_domain_ ## yDomain); \
    break;

static inline void set_yaxis(uint16_t table_key, void* pTable, uint16_t offset, byte value)
{
  CONCRETE_TABLE_ACTION(table_key, SET_YAXIS_VALUE, pTable, offset, value)
}

// Inline functions + compile time constants == fast division/modulus
#define OFFSET_TO_VALUE_INDEX(offset, size) (((size*size)-size)+(2*(offset % size))-offset)

#define GEN_GET_VALUE(size, xDomain, yDomain, pTable, offset) \
  return ((DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)*)pTable)->values[OFFSET_TO_VALUE_INDEX(offset, size)];

static inline table3d_value_t get_value(uint16_t table_key, void* pTable, uint16_t offset)
{
   CONCRETE_TABLE_ACTION(table_key, GEN_GET_VALUE, pTable, offset);
}

#define GEN_SET_VALUE(size, xDomain, yDomain, pTable, offset, value) \
  ((DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)*)pTable)->values[OFFSET_TO_VALUE_INDEX(offset, size)] = value; \
  break;

static inline void set_value(uint16_t table_key, void* pTable, uint16_t offset, table3d_value_t value)
{
   CONCRETE_TABLE_ACTION(table_key, GEN_SET_VALUE, pTable, offset, value);
}

#define GEN_INVALIDATE_CACHE(size, xDomain, yDomain, pTable) \
  invalidate_cache(&((DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)*)pTable)->get_value_cache); \
  break;

static inline void invalidate_cache(uint16_t table_key, void* pTable)
{
   CONCRETE_TABLE_ACTION(table_key, GEN_INVALIDATE_CACHE, pTable);
}

// Tables do not map linearly to the TS page address space, so special 
// handling is necessary (we do not use the normal array layout for
// performance reasons elsewhere)
//
// We take the offset & map it to a single value, x-axis or y-axis element
static inline byte get_table_value(const entity_t entity, uint16_t offset)
{
  uint8_t axisSize = table_key_to_axissize(entity.table_key);
  if (offset < TABLE_VALUE_END(axisSize))
  {
    return get_value(entity.table_key, entity.pEntity, offset);;
  } 
  if (offset < TABLE_AXISX_END(axisSize))
  {
    return get_xaxis(entity.table_key, entity.pEntity, offset);
  }
  return get_yaxis(entity.table_key, entity.pEntity, offset); 
}

static inline void set_table_value(const entity_t entity, uint16_t offset, int8_t value)
{
  uint8_t axisSize = table_key_to_axissize(entity.table_key);
  if (offset < TABLE_VALUE_END(axisSize))
  {
      set_value(entity.table_key, entity.pEntity, offset, value);
  }
  else if (offset < TABLE_AXISX_END(axisSize))
  {
      set_xaxis(entity.table_key, entity.pEntity, offset, value);
  }
  else 
  {
    set_yaxis(entity.table_key, entity.pEntity, offset, value);
  }
  invalidate_cache(entity.table_key, entity.pEntity);
}

static inline byte* get_raw_value(const entity_t &entity, uint16_t offset)
{
  return (byte*)entity.pEntity + offset;
}

// ============================ Page iteration support ======================

// Because the page iterators will not be called for every single byte
// inlining the mapping function is not performance critical.
//
// So save some memory.
static entity_t map_page_offset_to_entity(uint8_t pageNumber, uint16_t offset)
{
  return map_page_offset_to_entity_inline(pageNumber, offset);
}

static inline page_iterator_t to_page_entity(byte pageNum, entity_t mapped)
{
  return { mapped.pEntity ,
           .table_key = mapped.table_key, 
          .page=pageNum, .start = mapped.start, .size = mapped.size, .type = mapped.type };
}

// ====================================== External functions  ====================================

uint8_t getPageCount()
{
  return _countof(ini_page_sizes);
}

uint16_t getPageSize(byte pageNum)
{
  return ini_page_sizes[pageNum];
}

void setPageValue(byte pageNum, uint16_t offset, byte value)
{
  entity_t entity = map_page_offset_to_entity_inline(pageNum, offset);

  switch (entity.type)
  {
  case Table:
    set_table_value(entity, offset-entity.start, value);
    break;
  
  case Raw:
    *get_raw_value(entity, offset-entity.start) = value;
    break;
      
  default:
    break;
  }
}

byte getPageValue(byte page, uint16_t offset)
{
  entity_t entity = map_page_offset_to_entity_inline(page, offset);

  switch (entity.type)
  {
    case Table:
      return get_table_value(entity, offset-entity.start);
      break;

    case Raw:
      return *get_raw_value(entity, offset-entity.start);
      break;

    default: return 0U;
  }
  return 0U;
}

// Support iteration over a pages entities.
// Check for entity.type==End
page_iterator_t page_begin(byte pageNum)
{
  return to_page_entity(pageNum, map_page_offset_to_entity(pageNum, 0U));
}

page_iterator_t advance(const page_iterator_t &it)
{
    return to_page_entity(it.page, map_page_offset_to_entity(it.page, it.start+it.size));
}

#define GET_ROW_ITERATOR(size, xDomain, yDomain, pTable) \
    return rows_begin(((DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)*)pTable));

/**
 * Convert page iterator to table value iterator.
 */
table_row_iterator_t rows_begin(const page_iterator_t &it)
{
  CONCRETE_TABLE_ACTION(it.table_key, GET_ROW_ITERATOR, it.pData);
}

#define GET_X_ITERATOR(size, xDomain, yDomain, pTable) \
    return x_begin(((DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)*)pTable));

/**
 * Convert page iterator to table x axis iterator.
 */
table_axis_iterator_t x_begin(const page_iterator_t &it)
{
  CONCRETE_TABLE_ACTION(it.table_key, GET_X_ITERATOR, it.pData);
}

#define GET_Y_ITERATOR(size, xDomain, yDomain, pTable) \
    return y_begin(((DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)*)pTable));

/**
 * Convert page iterator to table y axis iterator.
 */
table_axis_iterator_t y_begin(const page_iterator_t &it)
{
  CONCRETE_TABLE_ACTION(it.table_key, GET_Y_ITERATOR, it.pData);
}