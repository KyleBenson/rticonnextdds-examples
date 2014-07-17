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
/* Shapes_publisher.cxx

 A publication of data of type ShapeType using DynamicData

 This file is derived from code automatically generated by the rtiddsgen
 command:

 rtiddsgen -language C++ -example <arch> Shapes.idl

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


 modification history
 ------------ -------
 */

#include <cstdio>
#include <cstdlib>
#ifdef RTI_VX653
#include <vThreadsData.h>
#endif
#include "Shapes.h"
#include "ndds/ndds_cpp.h"

#define EXAMPLE_TYPE_NAME "ShapesType"

/* Delete all entities */
static int publisher_shutdown(DDSDomainParticipant *participant,
        DDSDynamicDataTypeSupport *type_support) {
    DDS_ReturnCode_t retcode;
    int status = 0;

    if (participant != NULL) {
        if (type_support != NULL) {
            delete type_support;
            type_support = NULL;
        }

        retcode = participant->delete_contained_entities();
        if (retcode != DDS_RETCODE_OK) {
            printf("delete_contained_entities error %d\n", retcode);
            status = -1;
        }

        retcode = DDSTheParticipantFactory->delete_participant(participant);
        if (retcode != DDS_RETCODE_OK) {
            printf("delete_participant error %d\n", retcode);
            status = -1;
        }
    }

    /* RTI Connext provides finalize_instance() method on
     * domain participant factory for people who want to release memory used
     * by the participant factory. Uncomment the following block of code for
     * clean destruction of the singleton. */

    retcode = DDSDomainParticipantFactory::finalize_instance();
    if (retcode != DDS_RETCODE_OK) {
        printf("finalize_instance error %d\n", retcode);
        status = -1;
    }

    return status;
}

extern "C" int publisher_main(int domainId, int sample_count) {
    DDS_ReturnCode_t retcode;
    DDS_InstanceHandle_t instance_handle = DDS_HANDLE_NIL;
    const char *type_name = NULL;
    int count = 0;
    DDS_Duration_t send_period = { 0, 100000000 }; /* 100 ms */
    /*** Shape direction variables ***/
    int direction = 1; /* 1 means left to right and -1, right to left */
    int x_position = 50; /* 50 is the initial position */
    /*** DDS ENTITIES ***/
    DDSDomainParticipant *participant = NULL;
    DDSPublisher *publisher = NULL;
    DDSTopic *topic = NULL;
    DDSDataWriter *writer = NULL;
    /*** Dynamic Data parameters that we will need ***/
    struct DDS_TypeCode *type_code = NULL;
    DDS_DynamicData *data = NULL;
    DDS_Boolean data_is_initialized = DDS_BOOLEAN_FALSE;
    DDSDynamicDataWriter *DynamicData_writer = NULL;
    struct DDS_DynamicDataTypeProperty_t props;
    DDSDynamicDataTypeSupport *type_support = NULL;

    participant = DDSTheParticipantFactory->create_participant(domainId,
            DDS_PARTICIPANT_QOS_DEFAULT,
            NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        printf("create_participant error\n");
        publisher_shutdown(participant, type_support);
        return -1;
    }

    publisher = participant->create_publisher(DDS_PUBLISHER_QOS_DEFAULT,
    NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (publisher == NULL) {
        printf("create_publisher error\n");
        publisher_shutdown(participant, type_support);
        return -1;
    }

    /* Create DynamicData using TypeCode from Shapes.cxx 
     * If you are NOT using a type generated with rtiddsgen, you
     * need to create this TypeCode from scratch.
     */
    type_code = ShapeType_get_typecode();
    if (type_code == NULL) {
        printf("get_typecode error\n");
        publisher_shutdown(participant, type_support);
        return -1;
    }

    /* Create the Dynamic data type support object */
    type_support = new DDSDynamicDataTypeSupport(type_code, props);
    if (type_support == NULL) {
        printf("constructor DynamicDataTypeSupport error\n");
        publisher_shutdown(participant, type_support);
        return -1;
    }

    /* Register type before creating topic */
    type_name = EXAMPLE_TYPE_NAME;
    retcode = type_support->register_type(participant, EXAMPLE_TYPE_NAME);
    if (retcode != DDS_RETCODE_OK) {
        printf("register_type error\n");
        publisher_shutdown(participant, type_support);
        return -1;
    }

    /* Make sure both publisher and subscriber share the same topic 
     * name.
     * In the Shapes example: we are publishing a Square, which is the 
     * topic name. If you want to publish other shapes (Triangle or 
     * Circle), you just need to update the topic name. */
    topic = participant->create_topic("Square", type_name,
            DDS_TOPIC_QOS_DEFAULT, NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (topic == NULL) {
        printf("create_topic error\n");
        publisher_shutdown(participant, type_support);
        return -1;
    }

    /* First, we create a generic DataWriter for our topic */
    writer = publisher->create_datawriter(topic, DDS_DATAWRITER_QOS_DEFAULT,
    NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (writer == NULL) {
        printf("create_datawriter error\n");
        publisher_shutdown(participant, type_support);
        return -1;
    }

    /* Then, to use DynamicData, we need to assign the generic
     * DataWriter to a DynamicDataWriter, using 
     * DDS_DynamicDataWriter_narrow.
     * The following narrow function should never fail, as it performs 
     * only a safe cast. 
     */
    DynamicData_writer = DDSDynamicDataWriter::narrow(writer);
    if (DynamicData_writer == NULL) {
        printf("DynamicData_writer narrow error\n");
        publisher_shutdown(participant, type_support);
        return -1;
    }

    /* Create an instance of the sparse data we are about to send */
    data = type_support->create_data();

    if (data == NULL) {
        printf("create_data error\n");
        publisher_shutdown(participant, type_support);
        return -1;
    }

    /* Initialize the DynamicData object */
    data->set_string("color", DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, "BLUE");
    data->set_long("x", DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, 100);
    data->set_long("y", DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, 100);
    data->set_long("shapesize", DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, 30);

    /* Main loop */
    for (count = 0; (sample_count == 0) || (count < sample_count); ++count) {

        printf("Sending shapesize %d\n", 30 + (count % 20));
        printf("Sending x position %d\n", x_position);

        /* Modify the shapesize from 30 to 50 */
        retcode = data->set_long("shapesize",
                DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, 30 + (count % 20));
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "! Unable to set shapesize\n");
            publisher_shutdown(participant, type_support);
            return -1;
        }

        /* Modify the position */
        retcode = data->set_long("x", DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                x_position);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "! Unable to set x\n");
            publisher_shutdown(participant, type_support);
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
        retcode = DynamicData_writer->write(*data, instance_handle);
        if (retcode != DDS_RETCODE_OK) {
            printf("write error %d\n", retcode);
            publisher_shutdown(participant, type_support);
        }

        NDDSUtility::sleep(send_period);
    }

    /* Delete data sample */
    retcode = type_support->delete_data(data);
    if (retcode != DDS_RETCODE_OK) {
        printf("ShapeTypeTypeSupport::delete_data error %d\n", retcode);
    }

    /* Delete all entities */
    return publisher_shutdown(participant, type_support);
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
     NDDSConfigLogger::get_instance()->
     set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_API,
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
     NDDSConfigLogger::get_instance()->
     set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_API,
     NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
     */

    return publisher_main(domainId, sample_count);
}
#endif

#ifdef RTI_VX653
const unsigned char* __ctype = *(__ctypePtrGet());

extern "C" void usrAppInit ()
{
#ifdef  USER_APPL_INIT
    USER_APPL_INIT; /* for backwards compatibility */
#endif

    /* add application specific code here */
    taskSpawn("pub", RTI_OSAPI_THREAD_PRIORITY_NORMAL, 0x8, 0x150000,
            (FUNCPTR)publisher_main, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

}
#endif

