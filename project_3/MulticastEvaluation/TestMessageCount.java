import java.util.Iterator;
import java.util.List;

import org.jgroups.Address;
import org.jgroups.JChannel;
import org.jgroups.Message;
import org.jgroups.ReceiverAdapter;
import org.jgroups.View;
import org.jgroups.protocols.TP;

public class TestMessageCount extends ReceiverAdapter {
	JChannel channel;
	View currentView = null;
	Address leader;
	ExchangerState state = ExchangerState.STARTING;
	int recvMessageCount = 0;
	
	public enum ExchangerState {
		STARTING, RUNNING, ENDED;
	}
	
	public void viewAccepted(View view) {
		synchronized (this) {
			currentView = view;
			leader = currentView.getMembers().get(0);
		}
		System.out.println("** New view: " + currentView);
	}
	
	public void receive(Message msg) {
		recvMessageCount++;
		if(recvMessageCount == (currentView.getMembers().size() - 1) ) {
			state = ExchangerState.ENDED;
		}
	}

	public void mainLoop(int memberCount, int messageCount) throws Exception {
		long startTime = 0, stopTime = 0;
		channel = new JChannel();
		channel.setReceiver(this);
		channel.connect("MessageCountGroup");
		
		TP tp = channel.getProtocolStack().getTransport();
		System.out.println("** Transport layer TP: " + tp);
		System.out.println("** Total sent by TP: " + tp.getNumMessagesSent());
		System.out.println("** Total received by TP: " + tp.getNumMessagesReceived());
		state = ExchangerState.RUNNING;
		Address self = channel.getAddress();
		int curCount = 1;
		while(state != ExchangerState.ENDED) {
			curCount = currentView.getMembers().size();
			if(curCount == memberCount) {
				if(leader == self) {
					// Send messages to all members
					startTime = System.nanoTime();
					Message msg; 
					for(int j = 0; j < messageCount; j++) {
						msg = new Message(null, null, ("Message:" + j));
						channel.send(msg);
						System.out.println("** Sent Message count: " + j);
					}
					stopTime = System.nanoTime();
					Thread.sleep(1000*messageCount*memberCount);
					state = ExchangerState.ENDED;
				}						
			} else {
				Thread.sleep(500);	
			}		
		}
		
		long elapsedTime = stopTime - startTime;
		System.out.println("Out of main loop");
		System.out.println("** Transport layer TP: " + tp);
		System.out.println("** Total sent by TP: " + tp.getNumMessagesSent());
		System.out.println("** Total received by TP: " + tp.getNumMessagesReceived());
		System.out.println("** Elapsed time(nanosec) to send messages " + elapsedTime);
		channel.close();
		return;	
		
	}
	
	public static void main(String[] args) {
		if(args.length != 2) {
			System.out.println("java -cp .:jgroups-3.6.6.Final.jar TestMessageCount <memberCount> <messageCount>");
			System.exit(-1);
		}
		int noOfMembers = 0;
		int noOfMessages = 0;
		//try {
			noOfMembers = Integer.parseInt(args[0]);
			noOfMessages = Integer.parseInt(args[1]);
		//} catch ()
		System.setProperty("java.net.preferIPv4Stack", "true");
		TestMessageCount tmc = new TestMessageCount();
		try {
			tmc.mainLoop(noOfMembers, noOfMessages);
			return;
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}

