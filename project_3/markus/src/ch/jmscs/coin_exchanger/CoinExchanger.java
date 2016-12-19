package ch.jmscs.coin_exchanger;

import org.jgroups.*;
import org.jgroups.protocols.TP;

import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Random;
import java.util.concurrent.atomic.AtomicInteger;

public class CoinExchanger extends ReceiverAdapter {
    private int startingCoins;
    private int totalPeers;

    public enum ExchangerState {
        STARTING, RUNNING, ENDED
    }

    // print a message decorated with the local address
    private void log(String message) {
        message = channel.getAddressAsString() + ": " + message;
        System.out.println(message);
    }

    JChannel channel;
    View currentView = null, prevView = null;
    Address leader;
    AtomicInteger coins;
    HashMap<Address, AtomicInteger> unacknowledgedMessages = new HashMap<>();
    Random rand = new Random();
    ExchangerState state = ExchangerState.STARTING;

    public void viewAccepted(View view) {
        boolean updatedCoins = false;
        synchronized (this) {
            if (currentView != null) {
                prevView = currentView;
            } else {
                prevView = view;
            }
            currentView = view;
        }
        leader = currentView.getMembers().get(0);
        List<Address> leftMembers = View.leftMembers(prevView, currentView);
        for (Address removed : leftMembers) {
            if (unacknowledgedMessages.containsKey(removed)) {
                // synchronize to prevent errors transferring coins from unacknowledged to available
                synchronized (this) {
                    log("Adding unacknowledged coins back to coin count from client " + removed.toString());
                    coins.getAndAdd(unacknowledgedMessages.get(removed).getAndSet(0));
                    unacknowledgedMessages.remove(removed);
                    updatedCoins = true;
                }
            }
        }

        if (updatedCoins) {
            updateThreadState();
        }
        log("** New view: " + currentView);
    }

    public void receive(Message msg) {
        Address senderAddress = msg.getSrc();
        String msgStr = (String) msg.getObject();
        if (msgStr.contains("Req:")) {
            sendAck(senderAddress);
        } else if (msgStr.contains("Ack:")) {
            unacknowledgedMessages.get(senderAddress).decrementAndGet();

//            System.out.println("** Sent(with ack) coin to " +
//                    msg.getSrc() +
//                    ", now I have "
//                    + coins +
//                    ", unacknowledged coins "
//                    + sumOfUnacknowledgedCoins());
            updateThreadState();
        }
    }

    private int sumOfUnacknowledgedCoins() {
        return unacknowledgedMessages.values().stream().mapToInt(AtomicInteger::intValue).sum();
    }

    public void mainLoop() throws Exception {
        channel = new JChannel();
        channel.setReceiver(this);
        channel.connect("CoinExchangerGroup");
        TP tp = channel.getProtocolStack().getTransport();
//        log("** Transport layer TP: " + tp);
//        log("** Total sent by TP: " + tp.getNumMessagesSent());
//        log("** Total received by TP: " + tp.getNumMessagesReceived());
//        log("** Size of unack_ids: " + sumOfUnacknowledgedCoins());

        while (state != ExchangerState.ENDED) {
            sendCoin();
            Thread.sleep(100);
            updateThreadState();
        }
        System.out.println("** Total sent by TP: " + tp.getNumMessagesSent());
        System.out.println("** Total received by TP: " + tp.getNumMessagesReceived());
        System.out.println(String.format(
                "** Exiting, I have %1$d coins, %2$d unacknowledged coins, leaving %3$d total coins",
                coins.intValue(),
                sumOfUnacknowledgedCoins(),
                coins.intValue() + sumOfUnacknowledgedCoins())
        );
        channel.close();
        if (coins.intValue() != 0 && coins.intValue() != startingCoins * totalPeers) {
            throw new Exception("ILLEGAL STATE: STOPPED WITH COINS!");
        }
    }

    public void sendCoin() {
        List<Address> members = currentView.getMembers();
        if (members.size() > 1 && coins.get() > 0) {
            Address luckyGuy = members.get(rand.nextInt(members.size()));

            Message msg = new Message(luckyGuy, null, "Req: Coin");
            try {
                synchronized (this) {
                    coins.decrementAndGet();
                    if (!unacknowledgedMessages.containsKey(luckyGuy)) {
                        unacknowledgedMessages.put(luckyGuy, new AtomicInteger(0));
                    }
                    unacknowledgedMessages.get(luckyGuy).incrementAndGet();
                    channel.send(msg);
                }
//                System.out.println("** Sent coin to "
//                        + luckyGuy + ", now I have " + coins + ",  unacknowledged coins "
//                        + sumOfUnacknowledgedCoins());

            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public void sendAck(Address dst) {
        Message msg = new Message(dst, null, "Ack: Coin");
        boolean terminate = false;
        try {
            if (this.state == ExchangerState.ENDED) {
                terminate = true;
            } else {
                channel.send(msg);
                synchronized (this) {
                    coins.getAndIncrement();
                }
            }
            if (!terminate) {
//                System.out.println("** Got coin from " + dst + ", now I have " + coins + ",  unacknowledged coins " + sumOfUnacknowledgedCoins());
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
        if ((coins.get() <= 0 && (sumOfUnacknowledgedCoins() == 0)) || hasInput) {
            state = ExchangerState.ENDED;
        }
    }

    public static void main(int startingCoins, int totalPeers) throws Exception {
        System.setProperty("java.net.preferIPv4Stack", "true");
        ch.jmscs.coin_exchanger.CoinExchanger ce = new ch.jmscs.coin_exchanger.CoinExchanger();
        try {
            ce.startingCoins = startingCoins;
            ce.totalPeers = totalPeers;
            ce.coins = new AtomicInteger(startingCoins);
            ce.mainLoop();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}

