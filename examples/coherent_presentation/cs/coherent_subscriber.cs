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
/* coherent_subscriber.cs

   A subscription example

   This file is derived from code automatically generated by the rtiddsgen 
   command:

   rtiddsgen -language C# -example <arch> coherent.idl

   Example subscription of type coherent automatically generated by 
   'rtiddsgen'. To test them, follow these steps:

   (1) Compile this file and the example publication.

   (2) Start the subscription with the command
       objs\<arch>\coherent_subscriber <domain_id> <sample_count>

   (3) Start the publication with the command
       objs\<arch>\coherent_publisher <domain_id> <sample_count>

   (4) [Optional] Specify the list of discovery initial peers and 
       multicast receive addresses via an environment variable or a file 
       (in the current working directory) called NDDS_DISCOVERY_PEERS. 

   You can run any number of publishers and subscribers programs, and can 
   add and remove them dynamically from the domain.
                                   
   Example:
        
       To run the example application on domain <domain_id>:
                          
       bin\<Debug|Release>\coherent_publisher <domain_id> <sample_count>  
       bin\<Debug|Release>\coherent_subscriber <domain_id> <sample_count>
              
       
modification history
------------ -------
*/

public class coherentSubscriber {

    // Start changes for coherent_presentation
    private const int STATENVALS = 6;
    private static int[] statevals = { 0, 0, 0, 0, 0, 0 };

    public static void print_state() {
        char c = 'a';
        for (int i = 0; i < STATENVALS; ++i) {
            Console.Write(" {0} = {1};", c++, statevals[i]);
        }
    }

    public static void set_state( char c, int value ) {
        int idx = c - 'a';
        if (idx < 0 || idx >= STATENVALS) {
            Console.WriteLine("error: invalid field '{0}'", c);
            return;
        }
        statevals[idx] = value;
    }

    // End changes for coherent_presentation

    public class coherentListener : DDS.DataReaderListener {

        public override void on_requested_deadline_missed(
            DDS.DataReader reader,
            ref DDS.RequestedDeadlineMissedStatus status ) { }

        public override void on_requested_incompatible_qos(
            DDS.DataReader reader,
            DDS.RequestedIncompatibleQosStatus status ) { }

        public override void on_sample_rejected(
            DDS.DataReader reader,
            ref DDS.SampleRejectedStatus status ) { }

        public override void on_liveliness_changed(
            DDS.DataReader reader,
            ref DDS.LivelinessChangedStatus status ) { }

        public override void on_sample_lost(
            DDS.DataReader reader,
            ref DDS.SampleLostStatus status ) { }

        public override void on_subscription_matched(
            DDS.DataReader reader,
            ref DDS.SubscriptionMatchedStatus status ) { }

        public override void on_data_available( DDS.DataReader reader ) {
            coherentDataReader coherent_reader =
                (coherentDataReader)reader;

            try {
                coherent_reader.take(
                    data_seq,
                    info_seq,
                    DDS.ResourceLimitsQosPolicy.LENGTH_UNLIMITED,
                    DDS.SampleStateKind.ANY_SAMPLE_STATE,
                    DDS.ViewStateKind.ANY_VIEW_STATE,
                    DDS.InstanceStateKind.ANY_INSTANCE_STATE);
            } catch (DDS.Retcode_NoData) {
                return;
            } catch (DDS.Exception e) {
                Console.WriteLine("take error {0}", e);
                return;
            }

            // Start changes for coherent_presentation

            //Firstly, process all samples
            int len = 0;
            System.Int32 data_length = data_seq.length;
            for (int i = 0; i < data_length; ++i) {
                if (info_seq.get_at(i).valid_data) {
                    len++;
                    set_state(data_seq.get_at(i).field,
                        data_seq.get_at(i).value);
                }
            }

            // Then, we print the results
            if (len > 0) {
                Console.WriteLine("Received {0} updates", len);
                print_state();
                Console.WriteLine("");
            }

            // End changes for coherent_presentation

            try {
                coherent_reader.return_loan(data_seq, info_seq);
            } catch (DDS.Exception e) {
                Console.WriteLine("return loan error {0}", e);
            }
        }

        public coherentListener() {
            data_seq = new coherentSeq();
            info_seq = new DDS.SampleInfoSeq();
        }

        private coherentSeq data_seq;
        private DDS.SampleInfoSeq info_seq;
    };

    public static void Main( string[] args ) {

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
            coherentSubscriber.subscribe(
                domain_id, sample_count);
        } catch (DDS.Exception) {
            Console.WriteLine("error in subscriber");
        }
    }

    static void subscribe( int domain_id, int sample_count ) {

        // --- Create participant --- //

        /* To customize the participant QoS, use 
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

        // --- Create subscriber --- //

        /* To customize the subscriber QoS, use 
           the configuration file USER_QOS_PROFILES.xml */
        DDS.Subscriber subscriber = participant.create_subscriber(
            DDS.DomainParticipant.SUBSCRIBER_QOS_DEFAULT,
            null /* listener */,
            DDS.StatusMask.STATUS_MASK_NONE);
        if (subscriber == null) {
            shutdown(participant);
            throw new ApplicationException("create_subscriber error");
        }

        /* If you want to change the DataWriter's QoS programmatically rather than
         * using the XML file, you will need to add the following lines to your
         * code and comment out the create_subscriber call above.
         */

        // Start changes for coherent_presentation
        /* Get default publisher QoS to customize */
/*        DDS.SubscriberQos subscriber_qos = new DDS.SubscriberQos();
        participant.get_default_subscriber_qos(subscriber_qos);

        subscriber_qos.presentation.access_scope =
              DDS.PresentationQosPolicyAccessScopeKind.TOPIC_PRESENTATION_QOS;
        subscriber_qos.presentation.coherent_access = true;

        DDS.Subscriber subscriber = participant.create_subscriber(
            subscriber_qos,
            null,
            DDS.StatusMask.STATUS_MASK_NONE);
        if (subscriber == null) {
            shutdown(participant);
            throw new ApplicationException("create_subscriber error");
        }

*/        // End changes for coherent_presentation

        // --- Create topic --- //

        /* Register the type before creating the topic */
        System.String type_name = coherentTypeSupport.get_type_name();
        try {
            coherentTypeSupport.register_type(
                participant, type_name);
        } catch (DDS.Exception e) {
            Console.WriteLine("register_type error {0}", e);
            shutdown(participant);
            throw e;
        }

        /* To customize the topic QoS, use 
           the configuration file USER_QOS_PROFILES.xml */
        DDS.Topic topic = participant.create_topic(
            "Example coherent",
            type_name,
            DDS.DomainParticipant.TOPIC_QOS_DEFAULT,
            null /* listener */,
            DDS.StatusMask.STATUS_MASK_NONE);
        if (topic == null) {
            shutdown(participant);
            throw new ApplicationException("create_topic error");
        }

        // --- Create reader --- //

        /* Create a data reader listener */
        coherentListener reader_listener =
            new coherentListener();

        /* To customize the data reader QoS, use 
           the configuration file USER_QOS_PROFILES.xml */
        DDS.DataReader reader = subscriber.create_datareader(
            topic,
            DDS.Subscriber.DATAREADER_QOS_DEFAULT,
            reader_listener,
            DDS.StatusMask.STATUS_MASK_ALL);
        if (reader == null) {
            shutdown(participant);
            reader_listener = null;
            throw new ApplicationException("create_datareader error");
        }

        // Start changes for coherent_presentation

        /* Get default datareader QoS to customize */
/*        DDS.DataReaderQos datareader_qos = new DDS.DataReaderQos();
        subscriber.get_default_datareader_qos(datareader_qos);

        datareader_qos.reliability.kind = 
            DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
        datareader_qos.history.depth = 10;

        DDS.DataReader reader = subscriber.create_datareader(
            topic,
            datareader_qos,
            reader_listener,
            DDS.StatusMask.STATUS_MASK_ALL);
        if (reader == null) {
            shutdown(participant);
            reader_listener = null;
            throw new ApplicationException("create_datareader error");
        }
*/        // End changes for coherent_presentation

        // --- Wait for data --- //

        /* Main loop */
        const System.Int32 receive_period = 4000; // milliseconds
        for (int count = 0;
             (sample_count == 0) || (count < sample_count);
             ++count) {

            System.Threading.Thread.Sleep(receive_period);
        }

        // --- Shutdown --- //

        /* Delete all entities */
        shutdown(participant);
        reader_listener = null;
    }


    static void shutdown(
        DDS.DomainParticipant participant ) {

        /* Delete all entities */

        if (participant != null) {
            participant.delete_contained_entities();
            DDS.DomainParticipantFactory.get_instance().delete_participant(
                ref participant);
        }

        /* RTI Connext provides finalize_instance() method on
           domain participant factory for users who want to release memory
           used by the participant factory. Uncomment the following block of
           code for clean destruction of the singleton. */
        /*
        try {
            DDS.DomainParticipantFactory.finalize_instance();
        }
        catch(DDS.Exception e) {
            Console.WriteLine("finalize_instance error {0}", e);
            throw e;
        }
        */
    }
}


