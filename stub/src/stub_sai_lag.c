#include "sai.h"
#include "stub_sai.h"
#include "assert.h"
#include <stdlib.h>

typedef struct _stub_lag_member_object_id_t {
    sai_uint8_t  object_type;
    sai_uint8_t  reserved;
    sai_uint16_t port;
    sai_uint32_t oid;
} stub_lag_member_object_id_t;

typedef struct _lag {
    sai_object_id_t lag_oid;
    sai_object_id_t *members;
    sai_uint16_t num_of_members;
} lag_t;

static sai_uint16_t lags_count = 0;
static lag_t *lags_array;

static lag_t *getLagFromArray(const sai_object_id_t *lag_oid)
{
    int i = 0;
    for (; i < lags_count; ++i)
    {
        if (lags_array[i].lag_oid == *lag_oid)
        {
            return &lags_array[i];
        }
    }

    return NULL;
}

static bool resizeMembersArr(lag_t *lag, sai_uint16_t newSize)
{
    sai_object_id_t *newArr = realloc(lag->members, newSize * sizeof(sai_object_id_t));
    if (!newArr)
    {
        return false;
    }
    lag->members = newArr;
    lag->num_of_members = newSize;

    return true;
}

static bool addLagMember(const sai_object_id_t *lag_oid, const sai_object_id_t *lag_member)
{
    lag_t *lag = getLagFromArray(lag_oid);
    if (!lag)
    {
        return false;
    }

    if (!resizeMembersArr(lag, lag->num_of_members + 1))
    {
        return false;
    }

    lag->members[lag->num_of_members - 1] = *lag_member;
    return true;
}

bool resizeLagsArray(sai_uint16_t newSize)
{
    lag_t *newArray = realloc(lags_array, newSize * sizeof(lag_t));
    if (!newArray)
    {
        return false;
    }
    lags_count = newSize;
    lags_array = newArray;

    return true;
}


static lag_t *addLagToArray(const sai_object_id_t *lag_id)
{
    if (!resizeLagsArray(lags_count + 1))
    {
        return NULL;
    }

    lag_t *newLag = &lags_array[lags_count - 1];

    newLag->lag_oid = *lag_id;
    newLag->members = NULL;
    newLag->num_of_members = 0;

    return newLag;
}

static bool freeLag(const sai_object_id_t *lag_oid)
{
    int i;
    for (i = 0; i < lags_count; ++i)
    {
        if (lags_array[i].lag_oid == *lag_oid)
        {
            if (lags_array[i].num_of_members > 0) {
                free(lags_array[i].members);
            }
            lags_array[i] = lags_array[lags_count - 1];
            return resizeLagsArray(lags_count - 1);
        }
    }

    return false;
}

sai_status_t stub_create_lag(
    _Out_ sai_object_id_t* lag_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    static int32_t next_lag_id = 1;
    sai_status_t status;

    status = stub_create_object(SAI_OBJECT_TYPE_LAG, next_lag_id++, lag_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot create a LAG OID\n");
    }

    if (!addLagToArray(lag_id))
    {
        return SAI_STATUS_FAILURE;
    }

    printf("CREATED LAG: 0x%lX\n", *lag_id);
    return status;
}

sai_status_t stub_remove_lag(
    _In_ sai_object_id_t  lag_id)
{
    printf("REMOVED LAG: 0x%lX\n", lag_id);

    freeLag(&lag_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_set_lag_attribute(
    _In_ sai_object_id_t  lag_id,
    _In_ const sai_attribute_t *attr)
{
    printf("SET LAG ATTRIBUTE: 0x%lX\n", lag_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_get_lag_attribute(
    _In_ sai_object_id_t lag_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    const sai_attribute_value_t *unused;
    uint32_t indx;
    uint32_t i;

    printf("GET LAG ATTRIBUTE: 0x%lX\n", lag_id);

    assert(SAI_STATUS_SUCCESS == find_attrib_in_list(attr_count, attr_list, SAI_LAG_ATTR_PORT_LIST, &unused, &indx));

    sai_object_id_t *members = NULL;
    sai_uint16_t members_count = 0;
    for (i = 0; i < lags_count; ++i)
    {
        if (lags_array[i].lag_oid == lag_id) {
            members = lags_array[i].members;
            members_count = lags_array[i].num_of_members;
            break;
        }
    }

    if (members_count == 0)
    {
        return SAI_STATUS_SUCCESS;
    }

    for (i = 0; i < members_count && i < attr_count; ++i)
    {
        stub_lag_member_object_id_t *member = (stub_lag_member_object_id_t *)(members + i);
        attr_list->id = SAI_LAG_ATTR_PORT_LIST;
        attr_list->value.oid = member->port;

        printf("   LAG MEMBER: 0x%lX, PORT: %u \n", members[i], member->port);
    }

    if (i < members_count)
    {
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_create_lag_member(
    _Out_ sai_object_id_t* lag_member_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    static int32_t next_lag_member = 1;
    sai_status_t status;
    const sai_attribute_value_t *lag_member_port;
    const sai_attribute_value_t *lag_oid;
    uint32_t indx;

    assert(SAI_STATUS_SUCCESS == find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_LAG_ID, &lag_oid, &indx));
    assert(SAI_STATUS_SUCCESS == find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_PORT_ID, &lag_member_port, &indx));
    status = stub_create_object(SAI_OBJECT_TYPE_LAG_MEMBER, next_lag_member++, lag_member_id);

    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot create a LAG MEMBER\n");
    }

    stub_lag_member_object_id_t *member = (stub_lag_member_object_id_t*)lag_member_id;
    member->port = lag_member_port->u16;

    printf("CREATED LAG MEMBER: 0x%lX\n", *lag_member_id);

    if (!addLagMember(&lag_oid->oid, lag_member_id))
    {
        return SAI_STATUS_FAILURE;
    }

    return status;
}

sai_status_t stub_remove_lag_member(
    _In_ sai_object_id_t  lag_member_id)
{
    printf("REMOVE LAG MEMBER: 0x%lX\n", lag_member_id);

    int i = 0;
    for (; i < lags_count; ++i)
    {
        int j = 0;
        lag_t *lag = &lags_array[i];
        while(j < lag->num_of_members)
        {
            if (lag->members[j] == lag_member_id)
            {
                lag->members[i] = lag->members[lag->num_of_members - 1];
                bool res = resizeMembersArr(lag, lag->num_of_members -1);
                return res;
            }
            j += 1;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_set_lag_member_attribute(
    _In_ sai_object_id_t  lag_member_id,
    _In_ const sai_attribute_t *attr)
{
    printf("SET LAG MEMBER ATTRIBUTE: 0x%lX\n", lag_member_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_get_lag_member_attribute(
    _In_ sai_object_id_t lag_member_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    printf("GET LAG MEMBER ATTRIBUTE: 0x%lX\n", lag_member_id);

    return SAI_STATUS_SUCCESS;
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