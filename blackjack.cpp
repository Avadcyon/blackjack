#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <sstream>

using namespace std;

const int MAX_RESPLITS = 3;

enum CardValue { TWO = 2, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE };

class Card {
public:
    CardValue value;
    char suit;
    bool revealed;

    Card(CardValue val, char s, bool rev = true) : value(val), suit(s), revealed(rev) {}

    int getPoints(int currentTotal = 0) const {
        if (!revealed) return 0;  // If card is not revealed, it doesn't contribute to points
        switch (value) {
        case JACK:
        case QUEEN:
        case KING:
            return 10;
        case ACE:
            return (currentTotal + 11 <= 21) ? 11 : 1;
        default:
            return value;
        }
    }

    vector<string> getVisual() const {
        if (!revealed) {
            return {
                " _____ ",
                "|XXXXX|",
                "|XX X |",
                "|XXXXX|",
                " ----- "
            };
        }

        string valueStr;
        switch (value) {
        case TWO: valueStr = "2"; break;
        case THREE: valueStr = "3"; break;
        case FOUR: valueStr = "4"; break;
        case FIVE: valueStr = "5"; break;
        case SIX: valueStr = "6"; break;
        case SEVEN: valueStr = "7"; break;
        case EIGHT: valueStr = "8"; break;
        case NINE: valueStr = "9"; break;
        case TEN: valueStr = "T"; break;
        case JACK: valueStr = "J"; break;
        case QUEEN: valueStr = "Q"; break;
        case KING: valueStr = "K"; break;
        case ACE: valueStr = "A"; break;
        }
        return {
            " _____ ",
            "|" + valueStr + "    |",
            "|  " + string(1, suit) + "  |",
            "|    " + valueStr + "|",
            " ----- "
        };
    }
};

class Deck {
public:
    vector<Card> cards;

    Deck() {
        for (int i = 0; i < 6; ++i) { // 6 decks of 52 cards
            for (int j = TWO; j <= ACE; ++j) {
                for (int k = 0; k < 4; ++k) {
                    cards.push_back(Card(static_cast<CardValue>(j), randomSuit()));
                }
            }
        }
        shuffle();
    }

    void shuffle() {
        srand(static_cast<unsigned int>(time(0)));
        random_shuffle(cards.begin(), cards.end());
    }

    Card drawCard(bool reveal = true) {
        Card card = cards.back();
        cards.pop_back();
        card.revealed = reveal;
        return card;
    }

private:
    char randomSuit() {
        const char suits[] = { 'H', 'D', 'C', 'S' }; // Hearts, Diamonds, Clubs, Spades
        return suits[rand() % 4];
    }
};

class Player {
public:
    vector<vector<Card>> hands;

    Player() {
        hands.push_back(vector<Card>());
    }

    void drawCard(Deck& deck, int handIndex = 0, bool reveal = true) {
        if (handIndex >= hands.size()) {
            cout << "Invalid hand index." << endl;
            return;
        }
        if (hands[handIndex].size() >= 5) {
            cout << "Cannot draw more than 5 cards." << endl;
            return;
        }
        hands[handIndex].push_back(deck.drawCard(reveal));
    }

    int getPoints(int handIndex = 0) const {
        if (handIndex >= hands.size()) return 0;

        int total = 0;
        int aces = 0;
        for (const Card& card : hands[handIndex]) {
            if (card.revealed) {
                if (card.value == ACE) {
                    aces++;
                    total += 11;
                }
                else {
                    total += card.getPoints(total);
                }
            }
        }
        while (total > 21 && aces > 0) {
            total -= 10;
            aces--;
        }
        return total;
    }

    void printHand(int handIndex = 0) const {
        if (handIndex >= hands.size()) {
            cout << "Invalid hand index." << endl;
            return;
        }

        vector<vector<string>> visuals;
        for (const Card& card : hands[handIndex]) {
            visuals.push_back(card.getVisual());
        }

        for (int line = 0; line < 5; ++line) {
            for (const auto& visual : visuals) {
                cout << visual[line] << " ";
            }
            cout << endl;
        }
        cout << "Total: " << getPoints(handIndex) << endl;
    }

    bool isBlackjack(int handIndex = 0) const {
        if (handIndex >= hands.size()) return false;
        return hands[handIndex].size() == 2 && getPoints(handIndex) == 21;
    }

    bool canSplit(int handIndex = 0) const {
        if (handIndex >= hands.size()) return false;
        const vector<Card>& hand = hands[handIndex];
        return hand.size() == 2 && hand[0].getPoints() == hand[1].getPoints();
    }

    void splitHand(Deck& deck, int handIndex) {
        if (handIndex >= hands.size()) return;
        if (!canSplit(handIndex)) return;

        vector<Card> firstHand = hands[handIndex];
        vector<Card> secondHand;
        secondHand.push_back(firstHand.back());
        firstHand.pop_back();
        firstHand.push_back(deck.drawCard());
        secondHand.push_back(deck.drawCard());
        hands[handIndex] = firstHand;
        hands.push_back(secondHand);
    }
};

class Dealer : public Player {
public:
    Dealer() {
        hands[0].push_back(Card(TWO, 'H', false)); // Placeholder card for initialization
        hands[0].push_back(Card(TWO, 'H', false)); // Placeholder card for initialization
    }

    void drawInitialCards(Deck& deck) {
        hands[0][0] = deck.drawCard(true); // Reveal the first card
        hands[0][1] = deck.drawCard(false); // Hide the second card initially
    }

    void play(Deck& deck) {
        hands[0][1].revealed = true; // Reveal the second card at the start of dealer's turn
        while (getPoints(0) < 17) {
            drawCard(deck, 0);
        }
    }

    void revealCards() {
        for (auto& card : hands[0]) {
            card.revealed = true;
        }
    }

    void printHand(bool reveal = true) const {
        // Print dealer's hand with the appropriate reveal parameter
        vector<vector<string>> visuals;
        for (const Card& card : hands[0]) {
            visuals.push_back(card.getVisual());
        }

        for (int line = 0; line < 5; ++line) {
            for (const auto& visual : visuals) {
                cout << visual[line] << " ";
            }
            cout << endl;
        }
        if (reveal) {
            cout << "Total: " << getPoints(0) << endl;
        }
    }
};

void splitPair(Player& player, int i, Deck& deck, bool& resplitPossible) {
    if (player.canSplit(i) && player.hands.size() < 4) { // Ensure maximum of 4 hands
        char choice;
        while (true) {
            cout << "You have a pair in hand " << i + 1 << ". Do you want to split? (y/n): ";
            cin >> choice;
            if (choice == 'y') {
                player.splitHand(deck, i);
                resplitPossible = true; // Check for resplit again after splitting
                for (int j = 0; j < player.hands.size(); ++j) {
                    cout << "Hand " << j + 1 << ": " << endl;
                    player.printHand(j);
                }
                break;
            }
            if (choice == 'n') {
                break;
            }
            else {
                continue;
            }
        }
    }
}

void playGame() {
    Deck deck;
    Player player;
    Dealer dealer;

    dealer.drawInitialCards(deck);

    cout << "Dealer's hand: " << endl;
    dealer.printHand(false); // Only show the first card

    player.drawCard(deck);
    player.drawCard(deck);
    cout << "Your hand: " << endl;
    player.printHand();

    bool resplitPossible = true;

    while (resplitPossible) {
        resplitPossible = false;
        int i = 0;
        int handsize = player.hands.size();
        while (i < handsize) {
            splitPair(player, i, deck, resplitPossible);
            ++i;
            handsize = player.hands.size();
        }
    }

    char choice;
    if (player.isBlackjack()) {
        cout << "Blackjack!" << endl;
    }
    else {
        for (int i = 0; i < player.hands.size(); ++i) {
            while (true) {
                cout << "Draw another card for hand " << i + 1 << "? (y/n): ";
                cin >> choice;
                if (choice == 'y') {
                    player.drawCard(deck, i);
                    cout << "Hand " << i + 1 << ": " << endl;
                    player.printHand(i);
                    if (player.getPoints(i) > 21) {
                        cout << "Hand " << i + 1 << " busted! You lose." << endl;
                        break;
                    }
                }
                if (choice == 'n') {
                    break;
                }
                else {
                    continue;
                }
            }
        }
    }

    dealer.play(deck);
    dealer.revealCards(); // Reveal all dealer's cards after player's actions
    cout << "Dealer's hand: " << endl;
    dealer.printHand(); // Now show the complete dealer's hand

    for (int i = 0; i < player.hands.size(); ++i) {
        int playerPoints = player.getPoints(i);
        int dealerPoints = dealer.getPoints();

        if (playerPoints > 21) {
            cout << "Hand " << i + 1 << " busted! You lose." << endl;
        }
        else if (dealerPoints > 21 || playerPoints > dealerPoints) {
            cout << "Hand " << i + 1 << " wins!" << endl;
        }
        else if (playerPoints < dealerPoints) {
            cout << "Hand " << i + 1 << " loses." << endl;
        }
        else {
            cout << "Hand " << i + 1 << " ties." << endl;
        }
    }
}

int main() {
    char choiceplay;
    while (true) {
        cout << "\nWanna play blackjack? (y/n): ";
        cin >> choiceplay;
        if (choiceplay == 'y') {
            playGame();
        }
        else if (choiceplay == 'n') {
            break;
        }
        else {
            continue;
        }
    }
}
