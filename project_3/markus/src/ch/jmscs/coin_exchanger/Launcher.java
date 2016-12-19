package ch.jmscs.coin_exchanger;

import java.util.stream.IntStream;

public class Launcher {
    public static void main(String[] args) {
        int startingCoins = Integer.parseInt(args[0]);
        int totalPeers = Integer.parseInt(args[1]);
        IntStream.range(0, totalPeers).parallel().forEach(
                index ->
                        new Thread(
                                () -> {
                                    try {
                                        ch.jmscs.coin_exchanger.CoinExchanger.main(startingCoins, totalPeers);
                                    } catch (Exception e) {
                                        System.out.println(e);
                                    }
                                }
                        ).start()
        );
    }
}
