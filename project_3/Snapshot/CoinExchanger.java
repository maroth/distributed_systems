import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Collection;

import org.jgroups.Address;
import org.jgroups.JChannel;
import org.jgroups.Message;
import org.jgroups.ReceiverAdapter;
import org.jgroups.View;
import org.jgroups.protocols.TP;

public class CoinExchanger extends ReceiverAdapter {
	public enum ExchangerState {
		STARTING, RUNNING, ENDED;
	}
	
	public class Snapshot {
		String snapshotId; // Derived from initiator process's address 
		ExchangerState snapState = ExchangerState.STARTING; 
		int coins = 0;			// No. of coins 
		List<Address> unack_ids = null; // Current number list of unacknowledged coins
		Map<Address, Boolean> recordedChannelState;	// State of channel associted with given member
		Map<Address, Integer> coinMap;		// Coins in channel between address and self, i.e., (in transit) coins
		Map<Address, Integer> ackMap;		// Acknowledgements in channel between address and self, i.e., (in transit) acks
	}
	
	JChannel channel;
	View currentView = null, prevView = null;
	Address leader;
	Address myself;
	int coins = 3;
	Random rand = new Random();
	ExchangerState state = ExchangerState.STARTING;

	// Variables for termination algorithm
	List<Address> unack_ids;
	int unack_coins = 0;
		
	// Variables for Snapshot algorithm
	List<Snapshot> snapShots = null;
	boolean monitoring = false;
	boolean snapshot = false;
	
	public void viewAccepted(View view) {
		boolean updatedCoins = false;
		synchronized (this) {
			if(currentView != null) {
				prevView = currentView;
			} else {
				prevView = view;
			}
			currentView = view;
		}
		leader = currentView.getMembers().get(0);
		List<Address> leftMembers = View.leftMembers(prevView, currentView);
		if(!leftMembers.isEmpty()) {
			for (Iterator<Address> iter = leftMembers.iterator(); iter.hasNext(); ) {
				Address removed = iter.next();
				synchronized(this) {
			    		if((!unack_ids.isEmpty()) && unack_ids.remove(removed)) {
			    			coins++;
			    		}
					if(monitoring) {
						Collection<Boolean> vals = snapShots.get(0).recordedChannelState.values();
						if(!vals.contains(false)) {
							monitoring = false;
						}
					}
			    		
				}
			}
			updatedCoins = true;
		}
			
		if(updatedCoins) {
			updateThreadState();
		}
		System.out.println("** New view: " + currentView);
	}

	public void receive(Message msg) {
		Address senderAddress = msg.getSrc();
		String msgStr = (String)msg.getObject();
		if(msgStr.contains("Req:") ) {
			sendAck(senderAddress);
		} else if(msgStr.contains("Ack:")) {
			while(unack_ids.isEmpty());
			synchronized (this) {
				if(unack_ids.remove(senderAddress)) {
					unack_coins = unack_ids.size();
					if(monitoring && (snapShots.get(0).recordedChannelState.get(senderAddress) == false)) {
						if(snapShots.get(0).ackMap.containsKey(senderAddress)) {
							int oldVal = snapShots.get(0).ackMap.get(senderAddress);
							snapShots.get(0).ackMap.replace(senderAddress, oldVal, oldVal + 1);
						} else {
							snapShots.get(0).ackMap.put(senderAddress, 1);
						}						
					}
				}
			}
			System.out.println("** Sent(with ack) coin to " + msg.getSrc() + ", now I have " + coins + ", unacknowledged coins " + unack_ids.size());
			updateThreadState();
		} else if(msgStr.contains("Marker:")) {
			if(!monitoring) { // First Marker received. Start recording
				// Get snapshot id
				int i = msgStr.indexOf("Marker:");
				String snapId = msgStr.substring(i + 7);
				synchronized(this) {
					recordState(snapId.trim());
					sendMarkerToAll(null, msgStr);
					snapShots.get(0).recordedChannelState.put(senderAddress, true);
					monitoring = true;
					snapshot = true;
				}
			} else { // Record state of channel
				synchronized(this) {
					snapShots.get(0).recordedChannelState.put(senderAddress, true);
					Collection<Boolean> vals = snapShots.get(0).recordedChannelState.values();
					if(!vals.contains(false)) {
						monitoring = false;
					}
				}
			}
		}
			
	}

	public void mainLoop() throws Exception {
		unack_ids = new ArrayList<Address>();
		channel = new JChannel();
		channel.setReceiver(this);
		channel.connect("CoinExchangerGroup");
		TP tp = channel.getProtocolStack().getTransport();
		System.out.println("** Transport layer TP: " + tp);
		System.out.println("** Total sent by TP: " + tp.getNumMessagesSent());
		System.out.println("** Total received by TP: " + tp.getNumMessagesReceived());
		System.out.println("** Size of unack_ids: " + unack_ids.size());
		Address self = channel.getAddress();
		System.out.println("** Self address : " + self);

		while (state != ExchangerState.ENDED) {
			sendCoin();
			Thread.sleep(1000);
			updateThreadState();
			List<Address> members = currentView.getMembers();
			if(self.equals(leader)) {
				if(!snapshot && coins == 2) {
					System.out.println("\t** Snapshot started");
					snapshot = true; 
					startSnapshot();
				} 
			}
		}

		System.out.println("** Total sent by TP: " + tp.getNumMessagesSent());
		System.out.println("** Total received by TP: " + tp.getNumMessagesReceived());
		int total_coins = coins +  unack_ids.size() ;
		System.out.println("** Exiting, I have: " + coins + " coins " + "unack_coins " + unack_ids.size() + " total =" + total_coins+"\n");
		if(snapShots != null) {	
			System.out.println("** Printing Snapshot: Snapshot id:" + snapShots.get(0).snapshotId + " Coins: " + snapShots.get(0).coins);
			if(snapShots.get(0).unack_ids != null) {
				System.out.println("** Unacknowledged coins count = " + snapShots.get(0).unack_ids.size());
				for (Iterator<Address> iter = snapShots.get(0).unack_ids.iterator(); iter.hasNext(); ) {
					Address unack_id = iter.next();
					System.out.println("\t** " + unack_id);
				}
			} else {
				System.out.println("** Unacknowledged coins count =  0");
			}
				
			System.out.println("\t** Channel state ");
			if(snapShots.get(0).ackMap.isEmpty()) {
				System.out.println("\t** Acknowledgements = 0");
			} else {
				Iterator it = snapShots.get(0).ackMap.entrySet().iterator();
				while (it.hasNext()) {
					Map.Entry pair = (Map.Entry)it.next();
					System.out.println("\t** " + pair.getKey()  + " = " + pair.getValue());
				}
			}
			if(snapShots.get(0).coinMap.isEmpty()) {
				System.out.println("\t** Coins = 0");
			} else {
				System.out.println("\t** Coins");
				Iterator it2 = snapShots.get(0).coinMap.entrySet().iterator();
				while (it2.hasNext()) {
					Map.Entry pair = (Map.Entry)it2.next();
					System.out.println("\t** " + pair.getKey() + " = " + pair.getValue());
				}
			}
		}
			
		
		channel.close();
	}

	public void sendCoin() {
		List<Address> members = currentView.getMembers();
		if(members.size() > 1 && coins > 0) {
			Address luckyGuy = members.get(rand.nextInt(members.size()));

			Message msg = new Message(luckyGuy, null, "Req: Coin");
			try {
				channel.send(msg);
				synchronized (this) {
					coins--;
					unack_ids.add(luckyGuy);
					unack_coins++;
				}
				System.out.println("** Sent coin to " + luckyGuy + ", now I have " + coins + ",  unacknowledged coins " + unack_ids.size());
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	public void sendAck(Address dst) {
		Message msg = new Message(dst, null, "Ack: Coin");
		boolean terminate = false;
		if(monitoring) {
			synchronized(this) {
				if(snapShots.get(0).recordedChannelState.get(dst) == false) {
					if(snapShots.get(0).coinMap.containsKey(dst)) {
						int oldVal = snapShots.get(0).coinMap.get(dst);
						snapShots.get(0).coinMap.replace(dst, oldVal, oldVal + 1);
					} else {
						snapShots.get(0).coinMap.put(dst, 1);
					}						
				}
			}
		}						
		try {
			if(this.state == ExchangerState.ENDED) {
				terminate = true;
			} else {
				channel.send(msg);
				synchronized (this) {
					coins++;
				}
			}
			if(!terminate) {
				System.out.println("** Got coin from " + dst + ", now I have " + coins + ",  unacknowledged coins " + unack_ids.size());
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

	}

	public synchronized void updateThreadState() {
		if (currentView.getMembers().size() > 1) {
			state = ExchangerState.RUNNING;
		} else {
			if (state == ExchangerState.RUNNING) {
				state = ExchangerState.ENDED;
			}
		}
		boolean hasInput = false;
		try {
			hasInput = (System.in.available() > 0);
		} catch (IOException e) {
		}
		if ((coins <= 0 && (unack_ids.size()==0)) || hasInput) {
			state = ExchangerState.ENDED;
		}
	}
	
	public void recordState(String snapShotId) 
	{
		if(snapShots == null) {
			snapShots = new ArrayList<Snapshot>();
		}
		Snapshot curSnap = new Snapshot();
		curSnap.snapshotId = snapShotId;
		curSnap.coins = coins;
		if(unack_ids.size() > 0) {
			curSnap.unack_ids.addAll(unack_ids);
		}
		curSnap.snapState = ExchangerState.RUNNING;
		curSnap.coinMap = new HashMap<Address, Integer>();
		curSnap.ackMap = new HashMap<Address, Integer>();
		curSnap.recordedChannelState = new HashMap<Address, Boolean>();
		snapShots.add(curSnap);
	}
	
	public void startSnapshot()
	{
		Address self = channel.getAddress();
		
		synchronized(this) {
			monitoring = true;
			String snapShotId = "Snapshot" + self.toString();
			recordState(snapShotId);
			sendMarkerToAll(self, snapShotId);
		}
		
	}
	
	public void sendMarkerToAll(Address self, String snapshotId) {
		Address curMember;
		if(self == null)
			self = channel.getAddress();
		
		List<Address> members = currentView.getMembers();
		for(int i = 0; i < members.size(); i++) {
			curMember = members.get(i);
			if(curMember.equals(self)) {
				continue;
			}
			Message msg = new Message(curMember, null, ("Marker: " + snapshotId));
			try {
				snapShots.get(0).recordedChannelState.put(curMember, false);
				channel.send(msg);
				System.out.println("** Marker to " + curMember);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		
	}

	public static void main(String[] args) throws Exception {
		System.setProperty("java.net.preferIPv4Stack", "true");
		CoinExchanger ce = new CoinExchanger();
		try {
			ce.mainLoop();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}

