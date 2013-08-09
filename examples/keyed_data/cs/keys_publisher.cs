using System;
using System.Collections.Generic;
using System.Text;
/* keys_publisher.cs

   A publication of data of type keys

   This file is derived from code automatically generated by the rtiddsgen 
   command:

   rtiddsgen -language C# -example <arch> keys.idl

   Example publication of type keys automatically generated by 
   'rtiddsgen'. To test them follow these steps:

   (1) Compile this file and the example subscription.

   (2) Start the subscription with the command
       objs\<arch>\keys_subscriber <domain_id> <sample_count>
                
   (3) Start the publication with the command
       objs\<arch>\keys_publisher <domain_id> <sample_count>

   (4) [Optional] Specify the list of discovery initial peers and 
       multicast receive addresses via an environment variable or a file 
       (in the current working directory) called NDDS_DISCOVERY_PEERS. 

   You can run any number of publishers and subscribers programs, and can 
   add and remove them dynamically from the domain.


   Example:

       To run the example application on domain <domain_id>:

       bin\<Debug|Release>\keys_publisher <domain_id> <sample_count>
       bin\<Debug|Release>\keys_subscriber <domain_id> <sample_count>

       
modification history
------------ -------       
*/

public class keysPublisher {

    public static void Main(string[] args) {

        // --- Get domain ID --- //
        int domain_id = 0;
        if (args.Length >= 1) {
            domain_id = Int32.Parse(args[0]);
        }

        // --- Get max loop count; 0 means infinite loop  --- //
        int sample_count = 0;
        if (args.Length >= 2) {
            sample_count = Int32.Parse(args[1]);
        }

        /* Uncomment this to turn on additional logging
        NDDS.ConfigLogger.get_instance().set_verbosity_by_category(
            NDDS.LogCategory.NDDS_CONFIG_LOG_CATEGORY_API, 
            NDDS.LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
        */
    
        // --- Run --- //
        try {
            keysPublisher.publish(
                domain_id, sample_count);
        }
        catch(DDS.Exception)
        {
            Console.WriteLine("error in publisher");
        }
    }

    static void publish(int domain_id, int sample_count) {

        // --- Create participant --- //

        /* To customize participant QoS, use 
           the configuration file USER_QOS_PROFILES.xml */
        DDS.DomainParticipant participant =
            DDS.DomainParticipantFactory.get_instance().create_participant(
                domain_id,
                DDS.DomainParticipantFactory.PARTICIPANT_QOS_DEFAULT, 
                null /* listener */,
                DDS.StatusMask.STATUS_MASK_NONE);
        if (participant == null) {
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
        if (publisher == null) {
            shutdown(participant);
            throw new ApplicationException("create_publisher error");
        }

        // --- Create topic --- //

        /* Register type before creating topic */
        System.String type_name = keysTypeSupport.get_type_name();
        try {
            keysTypeSupport.register_type(
                participant, type_name);
        }
        catch(DDS.Exception e) {
            Console.WriteLine("register_type error {0}", e);
            shutdown(participant);
            throw e;
        }

        /* To customize topic QoS, use 
           the configuration file USER_QOS_PROFILES.xml */
        DDS.Topic topic = participant.create_topic(
            "Example keys",
            type_name,
            DDS.DomainParticipant.TOPIC_QOS_DEFAULT,
            null /* listener */,
            DDS.StatusMask.STATUS_MASK_NONE);
        if (topic == null) {
            shutdown(participant);
            throw new ApplicationException("create_topic error");
        }

        // --- Create writer --- //

        /* To customize data writer QoS, use 
           the configuration file USER_QOS_PROFILES.xml */
        DDS.DataWriter writer = publisher.create_datawriter(
            topic,
            DDS.Publisher.DATAWRITER_QOS_DEFAULT,
            null /* listener */,
            DDS.StatusMask.STATUS_MASK_NONE);

        /* If you want to set the writer_data_lifecycle QoS settings
        * programmatically rather than using the XML, you will need to add
        * the following lines to your code and comment out the create_datawriter
        * call above.
        */
        /*DDS.DataWriterQos datawriter_qos = new DDS.DataWriterQos();
        try
        {
            publisher.get_default_datawriter_qos(datawriter_qos);
        }
        catch (DDS.Exception e)
        {
            Console.WriteLine("get_default_datawriter_qos error", e);
        }

        datawriter_qos.writer_data_lifecycle.autodispose_unregistered_instances = false;

        DDS.DataWriter writer = publisher.create_datawriter(
        topic,
        datawriter_qos, 
        null,
        DDS.StatusMask.STATUS_MASK_NONE);
        */

        if (writer == null) {
            shutdown(participant);
            throw new ApplicationException("create_datawriter error");
        }
        keysDataWriter keys_writer = (keysDataWriter)writer;

        // --- Write --- //

        /* For a data type that has a key, if the same instance is going to be
           written multiple times, initialize the key here
           and register the keyed instance prior to writing */
        DDS.InstanceHandle_t instance_handle = DDS.InstanceHandle_t.HANDLE_NIL;

        /* Creates three instances */
        keys[] instance = new keys[3] { null, null, null };

        /* Create data samples for writing */
        instance[0] = keysTypeSupport.create_data();
        instance[1] = keysTypeSupport.create_data();
        instance[2] = keysTypeSupport.create_data();

        if (instance[0] == null || instance[1] == null || instance[2] == null)
        {
            shutdown(participant);
            throw new ApplicationException(
                "keysTypeSupport::create_data error\n");
        }

        /* RTI Connext could examine the key fields each time it needs to determine
         * which data-instance is being modified.
         * However, for performance and semantic reasons, it is better
         * for your application to declare all the data-instances it intends to
         * modify�prior to actually writing any samples. This is known as registration.
         */

        /* In order to register the instances, we must set their associated keys first */
        instance[0].code = 0;
        instance[1].code = 1;
        instance[2].code = 2;

        /* Creates three handles for managing the registrations */
        DDS.InstanceHandle_t[] handle =
            new DDS.InstanceHandle_t[]
            {DDS.InstanceHandle_t.HANDLE_NIL, DDS.InstanceHandle_t.HANDLE_NIL, 
                DDS.InstanceHandle_t.HANDLE_NIL};

        /* The keys must have been set before making this call */
        Console.WriteLine("Registering instance {0}", instance[0].code);
        handle[0] = keys_writer.register_instance(instance[0]);

        /* Modify the data to be sent here */
        instance[0].x = 1000;
        instance[1].x = 2000;
        instance[2].x = 3000;

        /* We only will send data over the instances marked as active */
        int[] active = new int[] { 1, 0, 0 }; // Only send active tracks.

        /* Main loop */
        const System.Int32 send_period = 1000; // milliseconds
        for (int count=0;
             (sample_count == 0) || (count < sample_count);
             ++count) {
            //Console.WriteLine("Writing keys, count {0}", count);

            switch (count)
            {
                case 5:
                    { /* Start sending the second and third instances */
                        Console.WriteLine("----Registering instance {0}", instance[1].code);
                        Console.WriteLine("----Registering instance {0}", instance[2].code);
                        handle[1] = keys_writer.register_instance(instance[1]);
                        handle[2] = keys_writer.register_instance(instance[2]);
                        active[1] = 1;
                        active[2] = 1;
                    } break;
                case 10:
                    { /* Unregister the second instance */
                        Console.WriteLine("----Unregistering instance {0}", instance[1].code);
                        try
                        {
                            keys_writer.unregister_instance(instance[1], ref handle[1]);
                        }
                        catch (DDS.Exception e)
                        {
                            Console.WriteLine("unregister instance error {0}", e);
                        }

                        active[1] = 0;
                    } break;
                case 15:
                    { /* Dispose the third instance */
                        Console.WriteLine("----Disposing instance {0}", instance[2].code);
                        try
                        {
                            keys_writer.dispose(instance[2], ref handle[2]);
                        }
                        catch (DDS.Exception e)
                        {
                            Console.WriteLine("dispose instance error {0}", e);
                        }
                        active[2] = 0;
                    } break;
            }

            /* Modify the data to be sent here */
            instance[0].y = count;
            instance[1].y = count;
            instance[2].y = count;

            for (int i = 0; i < 3; ++i)
            {
                if (active[i] == 1)
                {
                    Console.WriteLine("Writing instance {0}, x: {1}, y: {2}",
                           instance[i].code, instance[i].x, instance[i].y);
                    try
                    {
                        keys_writer.write(instance[i], ref handle[i]);
                    }
                    catch (DDS.Exception e)
                    {
                        Console.WriteLine("write error {0}", e);
                    }
                }
            }

            System.Threading.Thread.Sleep(send_period);
        }

        // --- Shutdown --- //

        /* Delete data samples */
        try
        {
            keysTypeSupport.delete_data(instance[0]);
        }
        catch (DDS.Exception e)
        {
            Console.WriteLine("keysTypeSupport.delete_data error: {0}", e);
        }
        try
        {
            keysTypeSupport.delete_data(instance[1]);
        }
        catch (DDS.Exception e)
        {
            Console.WriteLine("keysTypeSupport.delete_data error: {0}", e);
        }
        try
        {
            keysTypeSupport.delete_data(instance[2]);
        }
        catch (DDS.Exception e)
        {
            Console.WriteLine("keysTypeSupport.delete_data error: {0}", e);
        }

        /* Delete all entities */
        shutdown(participant);
    }

    static void shutdown(
        DDS.DomainParticipant participant) {

        /* Delete all entities */

        if (participant != null) {
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

