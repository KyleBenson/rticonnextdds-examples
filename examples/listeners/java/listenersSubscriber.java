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

/* listenersSubscriber.java

   A publication of data of type listeners

   This file is derived from code automatically generated by the rtiddsgen 
   command:

   rtiddsgen -language java -example <arch> .idl

   Example publication of type listeners automatically generated by 
   'rtiddsgen' To test them follow these steps:

   (1) Compile this file and the example subscription.

   (2) Start the subscription on the same domain used for with the command
       java listenersSubscriber <domain_id> <sample_count>

   (3) Start the publication with the command
       java listenersPublisher <domain_id> <sample_count>

   (4) [Optional] Specify the list of discovery initial peers and 
       multicast receive addresses via an environment variable or a file 
       (in the current working directory) called NDDS_DISCOVERY_PEERS. 
       
   You can run any number of publishers and subscribers programs, and can 
   add and remove them dynamically from the domain.
              
                                   
   Example:
        
       To run the example application on domain <domain_id>:
            
       Ensure that $(NDDSHOME)/lib/<arch> is on the dynamic library path for
       Java.                       
       
        On UNIX systems: 
             add $(NDDSHOME)/lib/<arch> to the 'LD_LIBRARY_PATH' environment
             variable
                                         
        On Windows systems:
             add %NDDSHOME%\lib\<arch> to the 'Path' environment variable
                        

       Run the Java applications:
       
        java -Djava.ext.dirs=$NDDSHOME/class listenersPublisher <domain_id>

        java -Djava.ext.dirs=$NDDSHOME/class listenersSubscriber <domain_id>  
       
       
modification history
------------ -------   
*/

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Arrays;

import com.rti.dds.domain.*;
import com.rti.dds.infrastructure.*;
import com.rti.dds.subscription.*;
import com.rti.dds.topic.*;
import com.rti.ndds.config.*;

// ===========================================================================

public class listenersSubscriber {
    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------
    
    public static void main(String[] args) {
        // --- Get domain ID --- //
        int domainId = 0;
        if (args.length >= 1) {
            domainId = Integer.valueOf(args[0]).intValue();
        }
        
        // -- Get max loop count; 0 means infinite loop --- //
        int sampleCount = 0;
        if (args.length >= 2) {
            sampleCount = Integer.valueOf(args[1]).intValue();
        }
        
        
        /* Uncomment this to turn on additional logging
        Logger.get_instance().set_verbosity_by_category(
            LogCategory.NDDS_CONFIG_LOG_CATEGORY_API,
            LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
        */
        
        // --- Run --- //
        subscriberMain(domainId, sampleCount);
    }
    
    
    
    // -----------------------------------------------------------------------
    // Private Methods
    // -----------------------------------------------------------------------
    
    // --- Constructors: -----------------------------------------------------
    
    private listenersSubscriber() {
        super();
    }
    
    
    // -----------------------------------------------------------------------
    
    private static void subscriberMain(int domainId, int sampleCount) {

        DomainParticipant participant = null;
        Subscriber subscriber = null;
        Topic topic = null;
        DataReaderListener listener = null;
        DataReader reader = null;

        try {

            // --- Create participant --- //
    
	    ParticipantListener participant_listener = new ParticipantListener();

	    /* We associate the participant_listener to the participant and set the 
	     * status mask to get all the statuses */
	    participant = DomainParticipantFactory.TheParticipantFactory.
                create_participant(domainId, 
				   DomainParticipantFactory.PARTICIPANT_QOS_DEFAULT,
				   participant_listener /* listener */, 
				   StatusKind.STATUS_MASK_ALL /* get all statuses */);
            if (participant == null) {
                System.err.println("create_participant error\n");
                return;
            }                         

            // --- Create subscriber --- //
	    
	    SubscriberListener subsriber_listener = new SubscriberListener();
	    
	    /* Here we associate the subscriber listener to the subscriber and set the 
	     * status mask to get all the statuses */
            subscriber = participant.create_subscriber(DomainParticipant.SUBSCRIBER_QOS_DEFAULT, 
						       subsriber_listener /* listener */,
						       StatusKind.STATUS_MASK_ALL);
            if (subscriber == null) {
                System.err.println("create_subscriber error\n");
                return;
            }     
                
            // --- Create topic --- //
        
            /* Register type before creating topic */
            String typeName = listenersTypeSupport.get_type_name(); 
            listenersTypeSupport.register_type(participant, typeName);
    
            /* To customize topic QoS, use
               the configuration file USER_QOS_PROFILES.xml */
    
            topic = participant.create_topic(
                "Example listeners",
                typeName, DomainParticipant.TOPIC_QOS_DEFAULT,
                null /* listener */, StatusKind.STATUS_MASK_NONE);
            if (topic == null) {
                System.err.println("create_topic error\n");
                return;
            }                     
        
            // --- Create reader --- //

	    ReaderListener reader_listener = new ReaderListener();
	    
	    /* Here we associate the data reader listener to the reader.
	     * We just listen for liveliness changed and data available,
	     * since most specific listeners will get called */
	    reader = (listenersDataReader)
                subscriber.create_datareader(topic, 
					     Subscriber.DATAREADER_QOS_DEFAULT, 
					     reader_listener /* listener */,
					     StatusKind.LIVELINESS_CHANGED_STATUS | 
					     StatusKind.DATA_AVAILABLE_STATUS /* statuses */);
            if (reader == null) {
                System.err.println("create_datareader error\n");
                return;
            }                         
        
            // --- Wait for data --- //

            final long receivePeriodSec = 1;

            for (int count = 0;
                 (sampleCount == 0) || (count < sampleCount);
                 ++count) {
                try {
                    Thread.sleep(receivePeriodSec * 1000);  // in millisec
                } catch (InterruptedException ix) {
                    System.err.println("INTERRUPTED");
                    break;
                }
            }
        } finally {

            // --- Shutdown --- //

            if(participant != null) {
                participant.delete_contained_entities();

                DomainParticipantFactory.TheParticipantFactory.
                    delete_participant(participant);
            }
            /* RTI Connext provides the finalize_instance()
               method for users who want to release memory used by the
               participant factory singleton. Uncomment the following block of
               code for clean destruction of the participant factory
               singleton. */
            //DomainParticipantFactory.finalize_instance();
        }
    }
    
    // -----------------------------------------------------------------------
    // Private Types
    // -----------------------------------------------------------------------
    
    // =======================================================================
    private static class ParticipantListener extends DomainParticipantAdapter {
	public void on_requested_deadline_missed(
            DataReader dataReader,
            RequestedDeadlineMissedStatus status) 
	{
	    System.out.println("ParticipantListener: on_requested_deadline_missed()");
	}    
	
	public void on_requested_incompatible_qos(
            DataReader dataReader,
            RequestedIncompatibleQosStatus status) 
	{
	    System.out.println("ParticipantListener: on_requested_incompatible_qos()");
	}
    
	public void on_sample_rejected(
            DataReader dataReader,
            SampleRejectedStatus status) 
	{
	    System.out.println("ParticipantListener: on_sample_rejected()");
	}
	
	public void on_liveliness_changed(
            DataReader dataReader,
	    LivelinessChangedStatus status) 
	{
	    System.out.println("ParticipantListener: on_liveliness_changed()");
	}
   
	public void on_sample_lost(
            DataReader dataReader,
            SampleLostStatus status) 
	{
	    System.out.println("ParticipantListener: on_sample_lost()");
	}
	
	public void on_subscription_matched(
            DataReader dataReader,
            SubscriptionMatchedStatus status) 
	{
	    System.out.println("ParticipantListener: on_subscription_matched()");
	}
	
	public void on_data_available(DataReader dataReader) 
	{
	    System.out.println("ParticipantListener: on_data_available()");
	}

	public void on_data_on_readers(Subscriber subscriber) 
	{
	    System.out.println("ParticipantListener: on_data_on_readers()");
	    
	    // notify_datareaders() only calls on_data_available for
	    // DataReaders with unread samples
	    subscriber.notify_datareaders();
	}
	
	public void on_inconsistent_topic(
	    Topic topic,
	    InconsistentTopicStatus status) 
	{
	    System.out.println("ParticipantListener: on_inconsistent_topic()");
	}
    }

    private static class SubscriberListener extends SubscriberAdapter {
        public void on_requested_deadline_missed(
            DataReader dataReader,
            RequestedDeadlineMissedStatus status) 
	{
            System.out.println("SubscriberListener: on_requested_deadline_missed()");
        }    
   
        public void on_requested_incompatible_qos(
            DataReader dataReader,
            RequestedIncompatibleQosStatus status) 
	{
            System.out.println("SubscriberListener: on_requested_incompatible_qos()");
        }
    
        public void on_sample_rejected(
            DataReader dataReader,
            SampleRejectedStatus status) 
	{
            System.out.println("SubscriberListener: on_sample_rejected()");
        }

        public void on_liveliness_changed(
            DataReader dataReader,
            LivelinessChangedStatus status) 
	{
            System.out.println("SubscriberListener: on_liveliness_changed()");
        }
   
        public void on_subscription_matched(
            DataReader dataReader,
            SubscriptionMatchedStatus status) 
	{
            System.out.println("SubscriberListener: on_subscription_matched()");
        }

        public void on_data_available(DataReader dataReader) 
	{
            System.out.println("SubscriberListener: on_data_available()");
        }
	
	private int count = 0;
        
	public void on_data_on_readers(Subscriber subscriber) {
	    

            System.out.println("SubscriberListener: on_data_on_readers()");

            // notify_datareaders() only calls on_data_available for
            // DataReaders with unread samples
            subscriber.notify_datareaders();

            if (++count > 3) {
                int newmask = StatusKind.STATUS_MASK_ALL;
                // 'Unmask' DATA_ON_READERS status for listener
                newmask &= ~StatusKind.DATA_ON_READERS_STATUS;
                subscriber.set_listener(this, newmask);
                System.out.print("Unregistering SubscriberListener::on_data_on_readers()\n");
            }
        }
    }
    
    
    private static class ReaderListener extends DataReaderAdapter {
        public void on_requested_deadline_missed(
            DataReader dataReader,
            RequestedDeadlineMissedStatus status) 
	{
            System.out.println("ReaderListener: on_requested_deadline_missed()");
        }    
   
        public void on_requested_incompatible_qos(
            DataReader dataReader,
            RequestedIncompatibleQosStatus status) 
	{
            System.out.println("ReaderListener: on_requested_incompatible_qos()");
        }
    
        public void on_sample_rejected(
            DataReader dataReader,
            SampleRejectedStatus status) 
	{
            System.out.println("ReaderListener: on_sample_rejected()");
        }
    
        public void on_liveliness_changed(
            DataReader dataReader,
            LivelinessChangedStatus status) 
	{
            System.out.println("ReaderListener: on_liveliness_changed()");
            System.out.print("  Alive writers: " + status.alive_count + "\n");
        }

        public void on_sample_lost(
            DataReader dataReader,
            SampleLostStatus status) 
	{
            System.out.println("ReaderListener: on_sample_lost()");
        }   
    
        public void on_subscription_matched(
            DataReader dataReader,
            SubscriptionMatchedStatus status) 
	{
            System.out.println("ReaderListener: on_subscription_matched()");
        }

        public void on_data_available(
            DataReader dataReader) 
	{
            System.out.println("ReaderListener: on_data_available()");

            listenersDataReader listenersReader =
                (listenersDataReader)dataReader;

            listenersSeq _dataSeq = new listenersSeq();
            SampleInfoSeq _infoSeq = new SampleInfoSeq();
            
            try {
                listenersReader.take(
                    _dataSeq, _infoSeq,
                    ResourceLimitsQosPolicy.LENGTH_UNLIMITED,
                    SampleStateKind.ANY_SAMPLE_STATE,
                    ViewStateKind.ANY_VIEW_STATE,
                    InstanceStateKind.ANY_INSTANCE_STATE);

                for(int i = 0; i < _infoSeq.size(); ++i) {
                    SampleInfo info = (SampleInfo)_infoSeq.get(i);

                    if (info.valid_data) {
                        System.out.println("   x: " + 
                                           ((listeners)_dataSeq.get(i)).x);
                    } else {
                        System.out.print("   Got metadata\n");
                    }
                }
            } catch (RETCODE_NO_DATA noData) {
                // No data to process
            } finally {
                listenersReader.return_loan(_dataSeq, _infoSeq);
            }
        }  
    } 
}


        