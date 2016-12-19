package ch.jmscs.coin_exchanger.original;

import java.io.IOException;
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
	View currentView;
	Address leader;
	Address myself;
	int coins = 3;
	Random rand = new Random();
	ExchangerState state = ExchangerState.STARTING;

	public void viewAccepted(View view) {
		synchronized (this) {
			currentView = view;
			leader = currentView.getMembers().get(0);
		}
		System.out.println("** New view: " + currentView);
	}

	public void receive(Message msg) {
		synchronized (this) {
			coins++;
		}
		System.out.println("** Got coin from " + msg.getSrc() + ", now I have " + coins);
	}

	public void mainLoop() throws Exception {
		channel = new JChannel();
		channel.setReceiver(this);
		channel.connect("CoinExchangerGroup");
		TP tp = channel.getProtocolStack().getTransport();
		System.out.println("** Transport layer TP: " + tp);
		System.out.println("** Total sent by TP: " + tp.getNumMessagesSent());
		System.out.println("** Total received by TP: " + tp.getNumMessagesReceived());

		while (state != ExchangerState.ENDED) {
			sendCoin();
			updateThreadState();
			Thread.sleep(1000);
		}
		System.out.println("** Total sent by TP: " + tp.getNumMessagesSent());
		System.out.println("** Total received by TP: " + tp.getNumMessagesReceived());
		System.out.println("** Exiting, I have: " + coins + " coins");
		channel.close();
	}

	public void sendCoin() {
		List<Address> members = currentView.getMembers();
		Address luckyGuy = members.get(rand.nextInt(members.size()));

		Message msg = new Message(luckyGuy, null, "Coin");
		try {
			channel.send(msg);
			synchronized (this) {
				coins--;
			}
			System.out.println("** Sent coin to " + luckyGuy + ", now I have " + coins);
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
		if (coins <= 0 || hasInput) {
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