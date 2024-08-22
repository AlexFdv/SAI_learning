#include "sai.h"
#include "stub_sai.h"
#include "assert.h"
#include <stdlib.h>

sai_status_t get_lag_member_attribute(_In_ const sai_object_key_t   *key,
                                      _Inout_ sai_attribute_value_t *value,
                                      _In_ uint32_t                  attr_index,
                                      _Inout_ vendor_cache_t        *cache,
                                      void                          *arg);

static const sai_attribute_entry_t lag_attribs[] = {
    { SAI_LAG_ATTR_PORT_LIST, false, false, false, true,
      "List of ports in LAG", SAI_ATTR_VAL_TYPE_OBJLIST },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_attribute_entry_t lag_member_attribs[] = {
    { SAI_LAG_MEMBER_ATTR_LAG_ID, true, true, false, true,
      "LAG ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_LAG_MEMBER_ATTR_PORT_ID, true, true, false, true,
      "PORT ID", SAI_ATTR_VAL_TYPE_OID },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t lag_vendor_attribs[] = {
    { SAI_LAG_ATTR_PORT_LIST,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};

static const sai_vendor_attribute_entry_t lag_member_vendor_attribs[] = {
    { SAI_LAG_MEMBER_ATTR_LAG_ID,
      { true, false, false, true },
      { true, false, false, true },
      get_lag_member_attribute, (void*) SAI_LAG_MEMBER_ATTR_LAG_ID,
      NULL, NULL },
    { SAI_LAG_MEMBER_ATTR_PORT_ID,
      { true, false, false, true },
      { true, false, false, true },
      get_lag_member_attribute, (void*) SAI_LAG_MEMBER_ATTR_PORT_ID,
      NULL, NULL }
};

#define MAX_NUMBER_OF_LAG_MEMBERS 16
#define MAX_NUMBER_OF_LAGS 5

typedef struct _lag_member_db_entry_t {
    bool            is_ised;
    sai_object_id_t port_oid;
    sai_object_id_t lag_oid;
} lag_member_db_entry_t;

typedef struct _lag_db_entry_t {
    bool            is_used;
    sai_object_id_t members_ids[MAX_NUMBER_OF_LAG_MEMBERS];
} lag_db_entry_t;

typedef struct lag_db_t {
    lag_db_entry_t        lags[MAX_NUMBER_OF_LAGS];
    lag_member_db_entry_t members[MAX_NUMBER_OF_LAG_MEMBERS];
} lag_db;

static lag_db lags_array;

static sai_int8_t find_free_lag()
{
    sai_int8_t i;
    for (i = 0; i < MAX_NUMBER_OF_LAGS; ++i)
    {
        if (!lags_array.lags[i].is_used)
        {
            return i;
        }
    }

    return -1;
}

static sai_int8_t get_free_lag_member()
{
    sai_int8_t i;
    for (i = 0; i < MAX_NUMBER_OF_LAG_MEMBERS; ++i)
    {
        if (!lags_array.members[i].is_ised)
        {
            return i;
        }
    }

    return -1;
}

sai_status_t stub_create_lag(
    _Out_ sai_object_id_t* lag_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    sai_status_t status;

    status = check_attribs_metadata(attr_count, attr_list, lag_attribs, lag_vendor_attribs, SAI_OPERATION_CREATE);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed attributes check\n");
        return status;
    }

    sai_int8_t lag_indx = find_free_lag();
    if (lag_indx < 0)
    {
        printf("Cannot create LAG: limit is reached\n");
        return SAI_STATUS_FAILURE;
    }

    status = stub_create_object(SAI_OBJECT_TYPE_LAG, lag_indx, lag_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot create a LAG OID\n");
    }

    lags_array.lags[lag_indx].is_used = true;
    printf("Created on index %i\n", lag_indx);

    char list_str[MAX_LIST_VALUE_STR_LEN];
    sai_attr_list_to_str(attr_count, attr_list, lag_attribs, MAX_LIST_VALUE_STR_LEN, list_str);

    printf("CREATED LAG: 0x%08lX (%s)\n", *lag_id, list_str);
    return status;
}

static sai_int8_t get_lag_index(sai_object_id_t lag_id)
{
    uint32_t lag_index;
    sai_status_t status = stub_object_to_type(lag_id, SAI_OBJECT_TYPE_LAG, &lag_index);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Cannot get LAG DB index.\n");
        return -1;
    }

    return lag_index;
}

static sai_int8_t get_lag_member_index(sai_object_id_t lag_member_id)
{
    uint32_t lag_member_index;
    sai_status_t status = stub_object_to_type(lag_member_id, SAI_OBJECT_TYPE_LAG_MEMBER, &lag_member_index);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot get LAG member DB index.\n");
        return -1;
    }

    return lag_member_index;
}

sai_status_t stub_remove_lag(_In_ sai_object_id_t  lag_id)
{
    sai_int8_t lag_index = get_lag_index(lag_id);
    if (lag_index < 0)
    {
        return SAI_STATUS_FAILURE;
    }

    lags_array.lags[lag_index].is_used = false;

    printf("REMOVED LAG: 0x%08lX\n", lag_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_set_lag_attribute(
    _In_ sai_object_id_t  lag_id,
    _In_ const sai_attribute_t *attr)
{
    printf("SET LAG ATTRIBUTE: 0x%08lX\n", lag_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_get_lag_attribute(
    _In_ sai_object_id_t lag_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    const sai_attribute_value_t *unused;
    uint32_t indx;
    sai_uint8_t i;
    uint32_t count = 0;

    assert(SAI_STATUS_SUCCESS == find_attrib_in_list(attr_count, attr_list, SAI_LAG_ATTR_PORT_LIST, &unused, &indx));

    for (i = 0; i < MAX_NUMBER_OF_LAG_MEMBERS && count < attr_list[indx].value.objlist.count; ++i)
    {
        if (lags_array.members[i].is_ised && lags_array.members[i].lag_oid == lag_id)
        {
            attr_list[indx].id = SAI_LAG_ATTR_PORT_LIST;
            attr_list[indx].value.objlist.list[count] = lags_array.members[i].port_oid;
            count++;
        }
    }

    // update count
    attr_list[indx].value.objlist.count = count;

    char list_str[MAX_LIST_VALUE_STR_LEN];
    sai_attr_list_to_str(1, attr_list + indx, lag_attribs, MAX_LIST_VALUE_STR_LEN, list_str);

    printf("GET LAG ATTRIBUTE: 0x%08lX (%s)\n", lag_id, list_str);

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_create_lag_member(
    _Out_ sai_object_id_t* lag_member_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    sai_status_t status;
    const sai_attribute_value_t *lag_member_port;
    const sai_attribute_value_t *lag_oid;
    uint32_t indx;

    sai_int8_t member_indx;
    sai_int8_t lag_index;

    status = check_attribs_metadata(attr_count, attr_list, lag_member_attribs, lag_member_vendor_attribs, SAI_OPERATION_CREATE);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed attributes check\n");
        return status;
    }

    assert(SAI_STATUS_SUCCESS == find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_LAG_ID, &lag_oid, &indx));
    assert(SAI_STATUS_SUCCESS == find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_PORT_ID, &lag_member_port, &indx));

    lag_index = get_lag_index(lag_oid->oid);
    if (lag_index < 0 || !lags_array.lags[lag_index].is_used)
    {
        printf("Failed to create a lag member: lag 0x%lX does not exist.\n", lag_oid->oid);
        return SAI_STATUS_FAILURE;
    }

    member_indx = get_free_lag_member();
    if (member_indx < 0)
    {
        printf("Failed to create a lag member: limit reached.\n");
        return SAI_STATUS_FAILURE;
    }

    status = stub_create_object(SAI_OBJECT_TYPE_LAG_MEMBER, member_indx, lag_member_id);

    lags_array.lags[lag_index].members_ids[member_indx] = *lag_member_id;
    lags_array.members[member_indx].is_ised = true;
    lags_array.members[member_indx].port_oid = lag_member_port->oid;
    lags_array.members[member_indx].lag_oid = lag_oid->oid;

    char list_str[MAX_LIST_VALUE_STR_LEN];
    sai_attr_list_to_str(attr_count, attr_list, lag_member_attribs, MAX_LIST_VALUE_STR_LEN, list_str);

    printf("CREATED LAG MEMBER: 0x%08lX (%s)\n", *lag_member_id, list_str);

     return status;
}

sai_status_t stub_remove_lag_member(
    _In_ sai_object_id_t  lag_member_id)
{
    sai_int8_t lag_indx;
    sai_int8_t lag_member_indx;
    printf("REMOVE LAG MEMBER: 0x%08lX\n", lag_member_id);

    lag_member_indx = get_lag_member_index(lag_member_id);
    if (lag_member_indx < 0)
    {
        return SAI_STATUS_FAILURE;
    }

    lags_array.members[lag_member_indx].is_ised = false;
    lag_indx = get_lag_index(lags_array.members[lag_member_indx].lag_oid);

    lags_array.lags[lag_indx].members_ids[lag_member_indx] = 0;
    memset(&lags_array.members[lag_member_indx], 0, sizeof(lag_member_db_entry_t));

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_set_lag_member_attribute(
    _In_ sai_object_id_t  lag_member_id,
    _In_ const sai_attribute_t *attr)
{
    printf("SET LAG MEMBER ATTRIBUTE: 0x%08lX\n", lag_member_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t get_lag_member_attribute(_In_ const sai_object_key_t   *key,
                                      _Inout_ sai_attribute_value_t *value,
                                      _In_ uint32_t                  attr_index,
                                      _Inout_ vendor_cache_t        *cache,
                                      void                          *arg)
{

    sai_int8_t lag_m_indx = get_lag_member_index(key->object_id);
    if (!lags_array.members[lag_m_indx].is_ised)
    {
        printf("Failed to get member attribute: lag member is not used.");
        return SAI_STATUS_FAILURE;
    }

    switch ((int64_t)arg) {
    case SAI_LAG_MEMBER_ATTR_LAG_ID:
        value->oid = lags_array.members[lag_m_indx].lag_oid;
        printf("Writing lag id 0x%08lX \n", value->oid);
        break;
    case SAI_LAG_MEMBER_ATTR_PORT_ID:
        value->oid = lags_array.members[lag_m_indx].port_oid;
        printf("Writing port id 0x%08lX \n", value->oid);
        break;
    default:
        printf("Got unexpected attribute ID\n");
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_get_lag_member_attribute(
    _In_ sai_object_id_t lag_member_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .object_id = lag_member_id };
    sai_status_t result = sai_get_attributes(&key, NULL, lag_member_attribs, lag_member_vendor_attribs, attr_count, attr_list);

    char list_str[MAX_LIST_VALUE_STR_LEN];
    sai_attr_list_to_str(attr_count, attr_list, lag_member_attribs, MAX_LIST_VALUE_STR_LEN, list_str);

    printf("GET LAG MEMBER ATTRIBUTE: 0x%08lX (%s)\n", lag_member_id, list_str);

    return result;
}

const sai_lag_api_t lag_api = {
    stub_create_lag,
    stub_remove_lag,
    stub_set_lag_attribute,
    stub_get_lag_attribute,
    stub_create_lag_member,
    stub_remove_lag_member,
    stub_set_lag_member_attribute,
    stub_get_lag_member_attribute
};