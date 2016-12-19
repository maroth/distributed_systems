package ch.jmscs.coin_exchanger;

import org.jgroups.*;
import org.jgroups.protocols.TP;

import java.util.stream.IntStream;

public class TestMessageCount extends ReceiverAdapter {
    JChannel channel;
    View currentView = null;
    Address leader;
    ExchangerState state = ExchangerState.STARTING;
    int recvMessageCount = 0;
    private int messageCount;

    public enum ExchangerState {
        STARTING, RUNNING, ENDED;
    }

    public void viewAccepted(View view) {
        synchronized (this) {
            currentView = view;
            leader = currentView.getMembers().get(0);
        }
//        System.out.println("** New view: " + currentView);
    }

    public void receive(Message msg) {
        recvMessageCount++;

        //end program after all messages were received
        if (recvMessageCount == messageCount) {
//            System.out.println("All messages were received, ending program.");
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            System.exit(0);
        }
    }

    public void mainLoop(int memberCount, int messageCount) throws Exception {
        this.messageCount = messageCount;
        channel = new JChannel();
        channel.setReceiver(this);
        channel.connect("MessageCountGroup");
        Address self = channel.getAddress();
        if (leader.equals(self)) {
            // wait for all members to become available
            while (currentView.getMembers().size() != memberCount) {
                Thread.sleep(100);
            }

            // wait to stabilize
            Thread.sleep(1000);
            long before = channel.getProtocolStack().getTransport().getNumMessagesSent();

            // send messages
            for (int j = 0; j < messageCount; j++) {
                Message msg = new Message(null, null, ("Message:" + j));
                channel.send(msg);
                System.out.println("** Sent Message count: " + j);
            }

            Thread.sleep(100);
            long count = channel.getProtocolStack().getTransport().getNumMessagesSent() - before;
            System.out.println("** Total sent by TP: " + count);

        }
    }

    public static void main(String[] args) {
        if (args.length != 2) {
            System.out.println("java -cp .:jgroups-3.6.6.Final.jar TestMessageCount <memberCount> <messageCount>");
            System.exit(-1);
        }
        final int noOfMembers = Integer.parseInt(args[0]);
        final int noOfMessages = Integer.parseInt(args[1]);
        System.out.println("Number of peers: " + noOfMembers);
        System.out.println("Number of messages : " + noOfMessages);
        System.setProperty("java.net.preferIPv4Stack", "true");
        IntStream.range(0, noOfMembers).forEach(index -> new Thread(() -> {
            try {
                new TestMessageCount().mainLoop(noOfMembers, noOfMessages);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }).start());
    }
}

