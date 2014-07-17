/*******************************************************************************
 (c) 2005-2014 Copyright, Real-Time Innovations, Inc.  All rights reserved.
 RTI grants Licensee a license to use, modify, compile, and create derivative
 works of the Software.  Licensee has the right to distribute object form only
 for use with RTI products.  The Software is provided "as is", with no warranty
 of any type, including any warranty for fitness for any purpose. RTI is under
 no obligation to maintain or support the Software.  RTI shall not be liable for
 any incidental or consequential damages arising out of the use or inability to
 use the software.
 ******************************************************************************/
/* Shapes_publisher.c

 A publication of data of type ShapeType using DynamicData

 This file is derived from code automatically generated by the rtiddsgen
 command:

 rtiddsgen -language C -example <arch> Shapes.idl

 If you are not using an IDL to generate the TypeCode,
 you will need to create it manually. Follow other DynamicData
 examples (such as NestedStruct one) to learn how to do it.

 Example:

 To run the example application on domain <domain_id>:

 On Unix:

 objs/<arch>/Shapes_publisher <domain_id> <sample #>
 objs/<arch>/Shapes_subscriber <domain_id> <sample #>

 On Windows:

 objs\<arch>\Shapes_publisher <domain_id> <sample #>
 objs\<arch>\Shapes_subscriber <domain_id> <sample #>

 */

#include <stdio.h>
#include <stdlib.h>
#include "ndds/ndds_c.h"
#include "Shapes.h"

#define EXAMPLE_TYPE_NAME "ShapesType"

/* Delete all created entities */
static int publisher_shutdown(DDS_DomainParticipant *participant,
        struct DDS_DynamicDataTypeSupport * type_support,
        DDS_Boolean data_is_initialized, struct DDS_DynamicData *data) {
    DDS_ReturnCode_t retcode;
    int status = 0;

    if (participant != NULL) {
        if (data_is_initialized) {
            DDS_DynamicData_finalize(data);
        }

        if (type_support != NULL) {
            DDS_DynamicDataTypeSupport_delete(type_support);
        }

        retcode = DDS_DomainParticipant_delete_contained_entities(participant);
        if (retcode != DDS_RETCODE_OK) {
            printf("delete_contained_entities error %d\n", retcode);
            status = -1;
        }

        retcode = DDS_DomainParticipantFactory_delete_participant(
                DDS_TheParticipantFactory, participant);
        if (retcode != DDS_RETCODE_OK) {
            printf("delete_participant error %d\n", retcode);
            status = -1;
        }
    }

    /* RTI Connext provides finalize_instance() method on
     * domain participant factory for people who want to release memory used
     * by the participant factory. Uncomment the following block of code for
     * clean destruction of the singleton. */

    retcode = DDS_DomainParticipantFactory_finalize_instance();
    if (retcode != DDS_RETCODE_OK) {
        printf("finalize_instance error %d\n", retcode);
        status = -1;
    }

    return status;
}

static int publisher_main(int domainId, int sample_count) {
    DDS_ReturnCode_t retcode;
    DDS_InstanceHandle_t instance_handle = DDS_HANDLE_NIL;
    const char *type_name = NULL;
    int count = 0;
    struct DDS_Duration_t send_period = { 0, 100000000 }; /* 100 ms */
    /*** Shape direction variables ***/
    int direction = 1; /* 1 means left to right and -1, right to left */
    int x_position = 50; /* 50 is the initial position */
    /*** DDS ENTITIES ***/
    DDS_DomainParticipant *participant = NULL;
    DDS_Publisher *publisher = NULL;
    DDS_Topic *topic = NULL;
    DDS_DataWriter *writer = NULL;
    /*** Dynamic Data parameters that we will need ***/
    struct DDS_TypeCode *type_code = NULL;
    struct DDS_DynamicData data;
    DDS_Boolean data_is_initialized = DDS_BOOLEAN_FALSE;
    DDS_DynamicDataWriter *DynamicData_writer = NULL;
    struct DDS_DynamicDataTypeProperty_t props =
            DDS_DynamicDataTypeProperty_t_INITIALIZER;
    struct DDS_DynamicDataTypeSupport *type_support = NULL;

    participant = DDS_DomainParticipantFactory_create_participant(
            DDS_TheParticipantFactory, domainId, &DDS_PARTICIPANT_QOS_DEFAULT,
            NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        printf("create_participant error\n");
        publisher_shutdown(participant, type_support, data_is_initialized,
                &data);
        return -1;
    }

    publisher = DDS_DomainParticipant_create_publisher(participant,
            &DDS_PUBLISHER_QOS_DEFAULT, NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (publisher == NULL) {
        printf("create_publisher error\n");
        publisher_shutdown(participant, type_support, data_is_initialized,
                &data);
        return -1;
    }

    /* Create DynamicData using TypeCode from Shapes.c 
     * If you are NOT using a type generated with rtiddsgen, you
     * need to create this TypeCode from scratch.
     */
    type_code = ShapeType_get_typecode();
    if (type_code == NULL) {
        printf("get_typecode error\n");
        publisher_shutdown(participant, type_support, data_is_initialized,
                &data);
        return -1;
    }

    /* Create the Dynamic Data TypeSupport object */
    type_support = DDS_DynamicDataTypeSupport_new(type_code, &props);
    if (type_support == NULL) {
        fprintf(stderr, "! Unable to create dynamic data type support\n");
        publisher_shutdown(participant, type_support, data_is_initialized,
                &data);
        return -1;
    }

    /* Register the type before creating the topic. */
    type_name = EXAMPLE_TYPE_NAME;
    retcode = DDS_DynamicDataTypeSupport_register_type(type_support,
            participant, type_name);
    if (retcode != DDS_RETCODE_OK) {
        publisher_shutdown(participant, type_support, data_is_initialized,
                &data);
        return -1;
    }

    /* Make sure both publisher and subscriber share the same topic 
     * name.
     * In the Shapes example: we are publishing a Square, which is the 
     * topic name. If you want to publish other shapes (Triangle or 
     * Circle), you just need to update the topic name. */
    topic = DDS_DomainParticipant_create_topic(participant, "Square", type_name,
            &DDS_TOPIC_QOS_DEFAULT, NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (topic == NULL) {
        printf("create_topic error\n");
        publisher_shutdown(participant, type_support, data_is_initialized,
                &data);
        return -1;
    }

    /* First, we create a generic DataWriter for our topic */
    writer = DDS_Publisher_create_datawriter(publisher, topic,
            &DDS_DATAWRITER_QOS_DEFAULT, NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (writer == NULL) {
        printf("create_datawriter error\n");
        publisher_shutdown(participant, type_support, data_is_initialized,
                &data);
        return -1;
    }

    /* Then, to use DynamicData, we need to assign the generic
     * DataWriter to a DynamicDataWriter, using 
     * DDS_DynamicDataWriter_narrow.
     * The following narrow function should never fail, as it performs 
     * only a safe cast. 
     */
    DynamicData_writer = DDS_DynamicDataWriter_narrow(writer);
    if (DynamicData_writer == NULL) {
        fprintf(stderr, "! Unable to narrow data writer into "
                "DDS_StringDataWriter\n");
        publisher_shutdown(participant, type_support, data_is_initialized,
                &data);
        return -1;
    }

    /* Create an instance of the sparse data we are about to send */
    data_is_initialized = DDS_DynamicData_initialize(&data, type_code,
            &DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
    if (!data_is_initialized) {
        printf("DynamicData_initialize error\n");
        publisher_shutdown(participant, type_support, data_is_initialized,
                &data);
        return -1;
    }

    /* Initialize the DynamicData object */
    DDS_DynamicData_set_string(&data, "color",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, "BLUE");
    DDS_DynamicData_set_long(&data, "x", DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            0);
    DDS_DynamicData_set_long(&data, "y", DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
            100);
    DDS_DynamicData_set_long(&data, "shapesize",
            DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, 30);

    /* Main loop */
    for (count = 0; (sample_count == 0) || (count < sample_count); ++count) {

        printf("Sending shapesize %d\n", 30 + (count % 20));
        printf("Sending x position %d\n", x_position);

        /* Modify the shapesize from 30 to 50 */
        retcode = DDS_DynamicData_set_long(&data, "shapesize",
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, 30 + (count % 20));
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "! Unable to set shapesize long\n");
            publisher_shutdown(participant, type_support, data_is_initialized,
                    &data);
            return -1;
        }
        /* Modify the position x from 50 to 150 */
        retcode = DDS_DynamicData_set_long(&data, "x",
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, x_position);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "! Unable to set x\n");
            publisher_shutdown(participant, type_support, data_is_initialized,
                    &data);
            return -1;
        }

        /* The x_position will be modified adding or substracting 
         * 2 to the previous x_position depending on the direction. 
         */
        x_position += (direction * 2);
        /* The x_position will stay between 50 and 150 pixels.
         * When the position is greater than 150 'direction' will be negative
         * (moving to the left) and when it is lower than 50 'direction' will 
         * be possitive (moving to the right).
         */
        if (x_position >= 150) {
            direction = -1;
        }
        if (x_position <= 50) {
            direction = 1;
        }

        /* Write data */
        retcode = DDS_DynamicDataWriter_write(DynamicData_writer, &data,
                &instance_handle);
        if (retcode != DDS_RETCODE_OK) {
            printf("write error %d\n", retcode);
            publisher_shutdown(participant, type_support, data_is_initialized,
                    &data);
        }

        NDDS_Utility_sleep(&send_period);
    }

    /* Cleanup and delete delete all entities */
    return publisher_shutdown(participant, type_support, data_is_initialized,
            &data);
}

#if defined(RTI_WINCE)
int wmain(int argc, wchar_t** argv)
{
    int domainId = 0;
    int sample_count = 0; /* infinite loop */

    if (argc >= 2) {
        domainId = _wtoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = _wtoi(argv[2]);
    }

    /* Uncomment this to turn on additional logging
     NDDS_Config_Logger_set_verbosity_by_category(
     NDDS_Config_Logger_get_instance(),
     NDDS_CONFIG_LOG_CATEGORY_API,
     NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
     */

    return publisher_main(domainId, sample_count);
}
#elif !(defined(RTI_VXWORKS) && !defined(__RTP__)) && !defined(RTI_PSOS)
int main(int argc, char *argv[]) {
    int domainId = 0;
    int sample_count = 0; /* infinite loop */

    if (argc >= 2) {
        domainId = atoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = atoi(argv[2]);
    }

    /* Uncomment this to turn on additional logging
     NDDS_Config_Logger_set_verbosity_by_category(
     NDDS_Config_Logger_get_instance(),
     NDDS_CONFIG_LOG_CATEGORY_API,
     NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
     */

    return publisher_main(domainId, sample_count);
}
#endif

#ifdef RTI_VX653
const unsigned char* __ctype = NULL;

void usrAppInit ()
{
#ifdef  USER_APPL_INIT
    USER_APPL_INIT; /* for backwards compatibility */
#endif

    /* add application specific code here */
    taskSpawn("pub", RTI_OSAPI_THREAD_PRIORITY_NORMAL, 0x8, 0x150000,
            (FUNCPTR)publisher_main, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

}
#endif
