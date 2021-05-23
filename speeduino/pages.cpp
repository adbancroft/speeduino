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
PROGMEM static constexpr const uint16_t ini_page_sizes[] = { 0, 128, 288, 288, 128, 288, 128, 240, 384, 192, 192, 288, 192, 128, 288 };

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

// 
#define _ENTITY_START(entityNum) entity ## entityNum ## Start
#define ENTITY_START_VAR(entityNum) _ENTITY_START(entityNum)
#define DECLARE_NEXT_ENTITY_START(entityIndex, entitySize) \
  /* Compute the start address of the next entity. We need this to be a constexpr */ \
  /* so we can static assert on it later. So we cannot increment an exiting var. */ \
  constexpr uint16_t ENTITY_START_VAR( PP_INC(entityIndex) ) = ENTITY_START_VAR(entityIndex)+entitySize;

// Signal the end of a page
#define END_OF_PAGE(pageNum, entityNum) \
  check_size<pageNum, ENTITY_START_VAR(entityNum)>(); \
  return create_entity(nullptr, 0, 0, ENTITY_START_VAR(entityNum), End);

// If the offset is in range, create a None entity_t
#define CHECK_NOENTITY(offset, blockSize, entityNum) \
  if (offset < ENTITY_START_VAR(entityNum)+blockSize) \
  { \
    return create_entity(nullptr, 0, ENTITY_START_VAR(entityNum), blockSize, NoEntity); \
  } \
  DECLARE_NEXT_ENTITY_START(entityNum, blockSize)

// If the offset is in range, create a Table entity_t
#define CHECK_TABLE(offset, pTable, entityNum) \
  if (offset < ENTITY_START_VAR(entityNum)+TABLE_SIZE(table_key_to_axissize((pTable)->type_key))) \
  { \
    return create_entity(pTable, (pTable)->type_key, ENTITY_START_VAR(entityNum), TABLE_SIZE(table_key_to_axissize((pTable)->type_key)), Table); \
  } \
  DECLARE_NEXT_ENTITY_START(entityNum, TABLE_SIZE(table_key_to_axissize((pTable)->type_key)))

// If the offset is in range, create a Raw entity_t
#define CHECK_RAW(offset, pDataBlock, blockSize, entityNum) \
  if (offset < ENTITY_START_VAR(entityNum)+blockSize) \
  { \
    return create_entity(pDataBlock, 0U, ENTITY_START_VAR(entityNum), blockSize, Raw); \
  } \
  DECLARE_NEXT_ENTITY_START(entityNum, blockSize)

// Does the heavy lifting of mapping page+offset to an entity
//
// Alternative implementation would be to encode the mapping into data structures
// That uses flash memory, which is scarce. And it was too slow.
static inline __attribute__((always_inline)) // <-- this is critical for performance
entity_t map_page_offset_to_entity_inline(uint8_t pageNumber, uint16_t offset)
{
  // The start address of the 1st entity in any page.
  static constexpr uint16_t ENTITY_START_VAR(0) = 0U;

  switch (pageNumber)
  {
    case 0:
      END_OF_PAGE(0, 0)

    case veMapPage:
    {
      CHECK_TABLE(offset, &fuelTable, 0)
      END_OF_PAGE(veMapPage, 1)
    }

    case ignMapPage: //Ignition settings page (Page 2)
    {
      CHECK_TABLE(offset, &ignitionTable, 0)
      END_OF_PAGE(ignMapPage, 1)
    }

    case afrMapPage: //Air/Fuel ratio target settings page
    {
      CHECK_TABLE(offset, &afrTable, 0)
      END_OF_PAGE(afrMapPage, 1)
    }

    case boostvvtPage: //Boost, VVT and staging maps (all 8x8)
    {
      CHECK_TABLE(offset, &boostTable, 0)
      CHECK_TABLE(offset, &vvtTable, 1)
      CHECK_TABLE(offset, &stagingTable, 2)
      END_OF_PAGE(boostvvtPage, 3)
    }

    case seqFuelPage:
    {
      CHECK_TABLE(offset, &trim1Table, 0)
      CHECK_TABLE(offset, &trim2Table, 1)
      CHECK_TABLE(offset, &trim3Table, 2)
      CHECK_TABLE(offset, &trim4Table, 3)
      CHECK_TABLE(offset, &trim5Table, 4)
      CHECK_TABLE(offset, &trim6Table, 5)
      CHECK_TABLE(offset, &trim7Table, 6)
      CHECK_TABLE(offset, &trim8Table, 7)
      END_OF_PAGE(seqFuelPage, 8)
    }

    case fuelMap2Page:
    {
      CHECK_TABLE(offset, &fuelTable2, 0)
      END_OF_PAGE(fuelMap2Page, 1)
    }

    case wmiMapPage:
    {
      CHECK_TABLE(offset, &wmiTable, 0)
      CHECK_TABLE(offset, &vvt2Table, 1)
      CHECK_TABLE(offset, &dwellTable, 2)
      END_OF_PAGE(wmiMapPage, 3)
    }
    
    case ignMap2Page:
    {
      CHECK_TABLE(offset, &ignitionTable2, 0)
      END_OF_PAGE(ignMap2Page, 1)
    }

    case veSetPage: 
    {
      CHECK_RAW(offset, &configPage2, sizeof(configPage2), 0)
      END_OF_PAGE(veSetPage, 1)
    }

    case ignSetPage: 
    {
      CHECK_RAW(offset, &configPage4, sizeof(configPage4), 0)
      END_OF_PAGE(ignSetPage, 1)
    }
    
    case afrSetPage: 
    {
      CHECK_RAW(offset, &configPage6, sizeof(configPage6), 0)
      END_OF_PAGE(afrSetPage, 1)
    }

    case canbusPage:  
    {
      CHECK_RAW(offset, &configPage9, sizeof(configPage9), 0)
      END_OF_PAGE(canbusPage, 1)
    }

    case warmupPage: 
    {
      CHECK_RAW(offset, &configPage10, sizeof(configPage10), 0)
      END_OF_PAGE(warmupPage, 1)
    }

    case progOutsPage: 
    {
      CHECK_RAW(offset, &configPage13, sizeof(configPage13), 0)
      END_OF_PAGE(progOutsPage, 1)
    }

    default:
      abort(); // Unkown page number. Not a lot we can do.
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

// Tables do not map linearly to the TS page address space, so special 
// handling is necessary (we do not use the normal array layout for
// performance reasons elsewhere)
//
// We take the offset & map it to a single value, x-axis or y-axis element

// Inline functions + compile time constants == fast division/modulus
#define OFFSET_TO_XAXIS_INDEX(offset, size) (offset - sq(size))
#define OFFSET_TO_YAXIS_INDEX(offset, size) ((size-1) - (offset - (sq(size)+size)))
#define OFFSET_TO_VALUE_INDEX(offset, size) (((size*size)-size)+(2*(offset % size))-offset)

#define GEN_GET_TABLE_VALUE(size, xDom, yDom) \
    static inline byte get_table_value(DECLARE_3DTABLE_TYPENAME(size, xDom, yDom) *pTable, uint16_t offset) \
    { \
      if (offset < TABLE_VALUE_END(size)) \
      { \
        return pTable->values[OFFSET_TO_VALUE_INDEX(offset, size)]; \
      } \
      if (offset < TABLE_AXISX_END(size)) \
      { \
        return pTable->axisX[OFFSET_TO_XAXIS_INDEX(offset, size)] / getTableAxisFactor(axis_domain_ ## xDom); \
      } \
      return pTable->axisY[OFFSET_TO_YAXIS_INDEX(offset, size)] / getTableAxisFactor(axis_domain_ ## yDom); \
    } 
TABLE_GENERATOR(GEN_GET_TABLE_VALUE)

static inline byte get_table_value(const entity_t entity, uint16_t offset)
{
  #define GEN_GET_VALUE(size, xDomain, yDomain, pTable, offset) \
    return get_table_value((DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)*)pTable, offset)
  CONCRETE_TABLE_ACTION(entity.table_key, GEN_GET_VALUE, entity.pEntity, offset);
}

#define GEN_SET_TABLE_VALUE(size, xDom, yDom) \
    static inline void set_table_value(DECLARE_3DTABLE_TYPENAME(size, xDom, yDom) *pTable, uint16_t offset, int8_t value) \
    { \
      if (offset < TABLE_VALUE_END(size)) \
      { \
        pTable->values[OFFSET_TO_VALUE_INDEX(offset, size)] = value; \
      } \
      else if (offset < TABLE_AXISX_END(size)) \
      { \
        pTable->axisX[OFFSET_TO_XAXIS_INDEX(offset, size)] = value * getTableAxisFactor(axis_domain_ ## xDom); \
      } \
      else \
      { \
        pTable->axisY[OFFSET_TO_YAXIS_INDEX(offset, size)] = value * getTableAxisFactor(axis_domain_ ## yDom); \
      } \
      invalidate_cache(&pTable->get_value_cache); \
    }
TABLE_GENERATOR(GEN_SET_TABLE_VALUE)

static inline void set_table_value(const entity_t entity, uint16_t offset, int8_t value)
{
  #define GEN_SET_VALUE(size, xDomain, yDomain, pTable, offset, value) \
    return set_table_value((DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)*)pTable, offset, value)
  CONCRETE_TABLE_ACTION(entity.table_key, GEN_SET_VALUE, entity.pEntity, offset, value);
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
  return pgm_read_word(ini_page_sizes + pageNum);
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

    case Raw:
      return *get_raw_value(entity, offset-entity.start);

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