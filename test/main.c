#include <stdio.h>
#include "sai.h"

#include <stdlib.h>

const char* test_profile_get_value(
    _In_ sai_switch_profile_id_t profile_id,
    _In_ const char* variable)
{
    return 0;
}

int test_profile_get_next_value(
    _In_ sai_switch_profile_id_t profile_id,
    _Out_ const char** variable,
    _Out_ const char** value)
{
    return -1;
}

const service_method_table_t test_services = {
    test_profile_get_value,
    test_profile_get_next_value
};

static bool getLagMembers(const sai_object_id_t *lag_oid, sai_lag_api_t *lag_api)
{
    sai_status_t status;
    sai_attribute_t attr_list[1];
    attr_list[0].id = SAI_LAG_ATTR_PORT_LIST;

    attr_list[0].value.objlist.count = 2;
    attr_list[0].value.objlist.list = calloc(1, sizeof(sai_object_id_t) * 2);

    status = lag_api->get_lag_attribute(*lag_oid, 2, attr_list);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed to create a LAG MEMBER, status=%d\n", status);
        return false;
    }

    free(attr_list[0].value.objlist.list);

    return true;
}

static bool getLagMemberAttributes(const sai_object_id_t *lag_member_oid, sai_lag_api_t *lag_api)
{
    sai_status_t status;
    sai_attribute_t attr_list[2];
    attr_list[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attr_list[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;

    status = lag_api->get_lag_member_attribute(*lag_member_oid, 2, attr_list);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed to create a LAG MEMBER, status=%d\n", status);
        return false;
    }

    return true;
}

int main()
{
    sai_status_t status;
    sai_lag_api_t *lag_api;
    sai_object_id_t lag_oid_1, lag_oid_2;
    sai_object_id_t lag_member[4];

    status = sai_api_initialize(0, &test_services);

    status = sai_api_query(SAI_API_LAG, (void**)&lag_api);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed to query LAG API, status=%d\n", status);
        return 1;
    }

    // LAG #1
    status = lag_api->create_lag(&lag_oid_1, 0, NULL);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed to create a LAG, status=%d\n", status);
        return 1;
    }
    // LAG MEMBER #1
    {
        sai_attribute_t attr_list[2];
        attr_list[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
        attr_list[0].value.oid = lag_oid_1;
        attr_list[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
        attr_list[1].value.oid = 0x010000001;
 
        status = lag_api->create_lag_member(&lag_member[0], 2, attr_list);
        if (status != SAI_STATUS_SUCCESS)
        {
            printf("Failed to create a LAG MEMBER, status=%d\n", status);
            return 1;
        }
    }
    // LAG MEMBER #2
    {
        sai_attribute_t attr_list[2];
        attr_list[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
        attr_list[0].value.oid = lag_oid_1;
        attr_list[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
        attr_list[1].value.oid = 0x010000002;

        status = lag_api->create_lag_member(&lag_member[1], 2, attr_list);
        if (status != SAI_STATUS_SUCCESS) {
            printf("Failed to create a LAG MEMBER, status=%d\n", status);
            return 1;
        }
    }

    // LAG #2
    status = lag_api->create_lag(&lag_oid_2, 0, NULL);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed to create a LAG, status=%d\n", status);
        return 1;
    }
    // LAG MEMBER #3
    {
        sai_attribute_t attr_list[2];
        attr_list[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
        attr_list[0].value.oid = lag_oid_2;
        attr_list[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
        attr_list[1].value.oid = 0x010000003;
 
        status = lag_api->create_lag_member(&lag_member[2], 2, attr_list);
        if (status != SAI_STATUS_SUCCESS)
        {
            printf("Failed to create a LAG MEMBER, status=%d\n", status);
            return 1;
        }
    }
    // LAG MEMBER #4
    {
        sai_attribute_t attr_list[2];
        attr_list[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
        attr_list[0].value.oid = lag_oid_2;
        attr_list[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
        attr_list[1].value.oid = attr_list[1].value.oid = 0x010000004;

        status = lag_api->create_lag_member(&lag_member[3], 2, attr_list);
        if (status != SAI_STATUS_SUCCESS) {
            printf("Failed to create a LAG MEMBER, status=%d\n", status);
            return 1;
        }
    }

    getLagMembers(&lag_oid_1, lag_api);
    getLagMembers(&lag_oid_2, lag_api);
    
    getLagMemberAttributes(&lag_member[0], lag_api);
    getLagMemberAttributes(&lag_member[1], lag_api);
    getLagMemberAttributes(&lag_member[2], lag_api);
    getLagMemberAttributes(&lag_member[3], lag_api);

    status = lag_api->remove_lag_member(lag_member[1]);
    status = lag_api->remove_lag_member(lag_member[2]);

    getLagMembers(&lag_oid_1, lag_api);
    getLagMembers(&lag_oid_2, lag_api);

    status = lag_api->remove_lag_member(lag_member[0]);
    status = lag_api->remove_lag_member(lag_member[3]);

    status = lag_api->remove_lag(lag_oid_1);
    status = lag_api->remove_lag(lag_oid_2);

    status = sai_api_uninitialize();

    return 0;
}