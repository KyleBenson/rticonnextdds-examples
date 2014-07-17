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
using System;
using System.Collections.Generic;
using System.Text;
/* listeners_publisher.cs

   A publication of data of type listeners

   This file is derived from code automatically generated by the rtiddsgen 
   command:

   rtiddsgen -language C# -example <arch> listeners.idl

   Example publication of type listeners automatically generated by 
   'rtiddsgen'. To test them follow these steps:

   (1) Compile this file and the example subscription.

   (2) Start the subscription with the command
       objs\<arch>\listeners_subscriber <domain_id> <sample_count>
                
   (3) Start the publication with the command
       objs\<arch>\listeners_publisher <domain_id> <sample_count>

   (4) [Optional] Specify the list of discovery initial peers and 
       multicast receive addresses via an environment variable or a file 
       (in the current working directory) called NDDS_DISCOVERY_PEERS. 

   You can run any number of publishers and subscribers programs, and can 
   add and remove them dynamically from the domain.


   Example:

       To run the example application on domain <domain_id>:

       bin\<Debug|Release>\listeners_publisher <domain_id> <sample_count>
       bin\<Debug|Release>\listeners_subscriber <domain_id> <sample_count>

       
modification history
------------ -------       
*/

public class listenersPublisher
{

    public class DataWriterListener : DDS.DataWriterListener
    {
        public override void on_offered_deadline_missed(
            DDS.DataWriter writer,
            ref DDS.OfferedDeadlineMissedStatus status) 
        {
            Console.Write("DataWriterListener: on_offered_deadline_missed");
        }

        public override void on_liveliness_lost(
            DDS.DataWriter writer,
            ref DDS.LivelinessLostStatus status) 
        {
            Console.Write("DataWriterListener: on_liveliness_lost");
        }

        public override void on_offered_incompatible_qos(
            DDS.DataWriter writer,
            DDS.OfferedIncompatibleQosStatus status)
        {
            Console.Write("DataWriterListener: on_offered_incompatible_qos");
        }

        public override void on_publication_matched(
            DDS.DataWriter writer,
            ref DDS.PublicationMatchedStatus status)
        {
            Console.WriteLine("DataWriterListener: on_publication_matched()");
            if (status.current_count_change < 0)
            {
                Console.WriteLine("lost a subscription");
            }
            else
            {
                Console.WriteLine("found a subscription");
            }
        }

        public override void on_reliable_writer_cache_changed(
            DDS.DataWriter writer,
            ref DDS.ReliableWriterCacheChangedStatus status) 
        {
            Console.WriteLine("DataWriterListener: on_reliable_writer_cache_changed()");  
        }

        public override void on_reliable_reader_activity_changed(
            DDS.DataWriter writer,
            ref DDS.ReliableReaderActivityChangedStatus status) 
        {
            Console.WriteLine("DataWriterListener: on_reliable_reader_activity_changed()");  
        }
    };

    public static void Main(string[] args)
    {

        // --- Get domain ID --- //
        int domain_id = 0;
        if (args.Length >= 1)
        {
            domain_id = Int32.Parse(args[0]);
        }

        // --- Get max loop count; 0 means infinite loop  --- //
        int sample_count = 0;
        if (args.Length >= 2)
        {
            sample_count = Int32.Parse(args[1]);
        }

        /* Uncomment this to turn on additional logging
        NDDS.ConfigLogger.get_instance().set_verbosity_by_category(
            NDDS.LogCategory.NDDS_CONFIG_LOG_CATEGORY_API, 
            NDDS.LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
        */

        // --- Run --- //
        try
        {
            listenersPublisher.publish(
                domain_id, sample_count);
        }
        catch (DDS.Exception)
        {
            Console.WriteLine("error in publisher");
        }
    }

    static void publish(int domain_id, int sample_count)
    {

        // --- Create participant --- //

        /* To customize participant QoS, use 
           the configuration file USER_QOS_PROFILES.xml */
        DDS.DomainParticipant participant =
            DDS.DomainParticipantFactory.get_instance().create_participant(
                domain_id,
                DDS.DomainParticipantFactory.PARTICIPANT_QOS_DEFAULT,
                null /* listener */,
                DDS.StatusMask.STATUS_MASK_NONE);
        if (participant == null)
        {
            shutdown(participant);
            throw new ApplicationException("create_participant error");
        }

        // --- Create publisher --- //

        /* To customize publisher QoS, use 
           the configuration file USER_QOS_PROFILES.xml */
        DDS.Publisher publisher = participant.create_publisher(
        DDS.DomainParticipant.PUBLISHER_QOS_DEFAULT,
        null /* listener */,
        DDS.StatusMask.STATUS_MASK_NONE);
        if (publisher == null)
        {
            shutdown(participant);
            throw new ApplicationException("create_publisher error");
        }

        /* Create ande Delete Inconsistent Topic 
         * ---------------------------------------------------------------
         * Here we create an inconsistent topic to trigger the subscriber 
         * application's callback. 
         * The inconsistent topic is created with the topic name used in 
         * the Subscriber application, but with a different data type -- 
         * the msg data type defined in partitions.idl.
         * Once it is created, we sleep to ensure the applications discover
         * each other and delete the Data Writer and Topic.
         */

        /* First we register the msg type -- we name it
         * inconsistent_topic_type_name to avoid confusion. 
         */
        Console.WriteLine("Creating Inconsistent Topic...");

        System.String inconsistent_type_name = msgTypeSupport.get_type_name();
        try
        {
            msgTypeSupport.register_type(participant, inconsistent_type_name);
        }
        catch (DDS.Exception e)
        {
            Console.WriteLine("register_type error {0}", e);
            shutdown(participant);
            throw e;
        }

        DDS.Topic inconsistent_topic = participant.create_topic(
            "Example listeners",
            inconsistent_type_name,
            DDS.DomainParticipant.TOPIC_QOS_DEFAULT,
            null /* listener */,
            DDS.StatusMask.STATUS_MASK_NONE);
        if (inconsistent_topic == null)
        {
            shutdown(participant);
            throw new ApplicationException("create_topic error");
        }

        /* We have to associate a writer to the topic, as Topic information is not
         * actually propagated until the creation of an associated writer.
         */
        DDS.DataWriter inconsistent_writer = publisher.create_datawriter(
            inconsistent_topic,
            DDS.Publisher.DATAWRITER_QOS_DEFAULT,
            null /* listener */,
            DDS.StatusMask.STATUS_MASK_NONE);
        if (inconsistent_writer == null)
        {
            shutdown(participant);
            throw new ApplicationException("create_datawriter error");
        }
        msgDataWriter msg_writer = (msgDataWriter)inconsistent_writer;

        // Sleep to leave time for applications to discover each other
        const System.Int32 sleep_period = 2000; //millseconds
        System.Threading.Thread.Sleep(sleep_period);

        try
        {
            publisher.delete_datawriter(ref inconsistent_writer);
        }
        catch (DDS.Exception e)
        {
            Console.WriteLine("delete_datawriter error {0}", e);
            shutdown(participant);
            return;
        }

        try
        {
            participant.delete_topic(ref inconsistent_topic);
        }
        catch (DDS.Exception e)
        {
            Console.WriteLine("delete_topic error {0}", e);
            shutdown(participant);
            return;
        }

        Console.WriteLine("... Deleted Inconsistent Topic\n");


        /* Create Consistent Topic 
         * -----------------------------------------------------------------
         * Once we have created the inconsistent topic with the wrong type, 
         * we create a topic with the right type name -- listeners -- that we 
         * will use to publish data. 
         */

        /* Register type before creating topic */
        System.String type_name = listenersTypeSupport.get_type_name();
        try
        {
            listenersTypeSupport.register_type(
                participant, type_name);
        }
        catch (DDS.Exception e)
        {
            Console.WriteLine("register_type error {0}", e);
            shutdown(participant);
            throw e;
        }

        /* To customize topic QoS, use 
           the configuration file USER_QOS_PROFILES.xml */
        DDS.Topic topic = participant.create_topic(
            "Example listeners",
            type_name,
            DDS.DomainParticipant.TOPIC_QOS_DEFAULT,
            null /* listener */,
            DDS.StatusMask.STATUS_MASK_NONE);
        if (topic == null)
        {
            shutdown(participant);
            throw new ApplicationException("create_topic error");
        }



        /* We will use the Data Writer Listener defined above to print
         * a message when some of events are triggered in the DataWriter. 
         * To do that, first we have to pass the writer_listener and then
         * we have to enable all status in the status mask.
         */
        DataWriterListener writer_listener = new DataWriterListener();
        DDS.DataWriter writer = publisher.create_datawriter(
            topic,
            DDS.Publisher.DATAWRITER_QOS_DEFAULT,
            writer_listener /* listener */,
            DDS.StatusMask.STATUS_MASK_ALL /* get all statuses */);
        if (writer == null)
        {
            shutdown(participant);
            writer_listener = null;
            throw new ApplicationException("create_datawriter error");
        }
        listenersDataWriter listeners_writer =
            (listenersDataWriter)writer;

        // --- Write --- //

        /* Create data sample for writing */
        listeners instance = listenersTypeSupport.create_data();
        if (instance == null)
        {
            shutdown(participant);
            writer_listener = null;
            throw new ApplicationException(
                "listenersTypeSupport.create_data error");
        }

        /* For a data type that has a key, if the same instance is going to be
           written multiple times, initialize the key here
           and register the keyed instance prior to writing */
        DDS.InstanceHandle_t instance_handle = DDS.InstanceHandle_t.HANDLE_NIL;
        /*
        instance_handle = listeners_writer.register_instance(instance);
        */

        /* Main loop */
        const System.Int32 send_period = 1000; // milliseconds
        for (int count = 0;
             (sample_count == 0) || (count < sample_count);
             ++count)
        {
            Console.WriteLine("Writing listeners, count {0}", count);

            /* Modify the data to be sent here */
            instance.x = (short) count;

            try
            {
                listeners_writer.write(instance, ref instance_handle);
            }
            catch (DDS.Exception e)
            {
                Console.WriteLine("write error {0}", e);
            }

            System.Threading.Thread.Sleep(send_period);
        }

        /*
        try {
            listeners_writer.unregister_instance(
                instance, ref instance_handle);
        } catch(DDS.Exception e) {
            Console.WriteLine("unregister instance error: {0}", e);
        }
        */

        // --- Shutdown --- //

        /* Delete data sample */
        try
        {
            listenersTypeSupport.delete_data(instance);
        }
        catch (DDS.Exception e)
        {
            Console.WriteLine(
                "listenersTypeSupport.delete_data error: {0}", e);
        }

        /* Delete all entities */
        shutdown(participant);
        writer_listener = null;
    }

    static void shutdown(
        DDS.DomainParticipant participant)
    {

        /* Delete all entities */

        if (participant != null)
        {
            participant.delete_contained_entities();
            DDS.DomainParticipantFactory.get_instance().delete_participant(
                ref participant);
        }

        /* RTI Connext provides finalize_instance() method on
           domain participant factory for people who want to release memory
           used by the participant factory. Uncomment the following block of
           code for clean destruction of the singleton. */
        /*
        try {
            DDS.DomainParticipantFactory.finalize_instance();
        } catch (DDS.Exception e) {
            Console.WriteLine("finalize_instance error: {0}", e);
            throw e;
        }
        */
    }
}

