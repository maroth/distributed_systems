import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Random;

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

	JChannel channel;
	View currentView = null, prevView = null;
	Address leader;
	Address myself;
	int coins = 3;
	List<Address> unack_ids;
	int unack_coins = 0;
	Random rand = new Random();
	ExchangerState state = ExchangerState.STARTING;

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
				}
			}
			System.out.println("** Sent(with ack) coin to " + msg.getSrc() + ", now I have " + coins + ", unacknowledged coins " + unack_ids.size());
			updateThreadState();
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

		while (state != ExchangerState.ENDED) {
			sendCoin();
			Thread.sleep(1000);
			updateThreadState();
		}
		System.out.println("** Total sent by TP: " + tp.getNumMessagesSent());
		System.out.println("** Total received by TP: " + tp.getNumMessagesReceived());
		int total_coins = coins +  unack_ids.size() ;
		System.out.println("** Exiting, I have: " + coins + " coins " + "unack_coins " + unack_ids.size() + " total =" + total_coins);
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
