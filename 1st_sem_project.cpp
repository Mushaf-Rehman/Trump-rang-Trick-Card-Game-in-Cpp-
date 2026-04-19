#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <ctime>

using namespace std;

string suits[4] = {"Hearts", "Diamonds", "Clubs", "Spades"};
string ranks[13] = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};

struct Card
{
    string rank;
    string suit;
};

int rank_index(const string &r)
{
    for (int i = 0; i < 13; i++)
    {
        if (ranks[i] == r)
        {
            return i;
        }
    }
    return -1;
}

void swap_card(Card &a, Card &b)
{
    Card t = a;
    a = b;
    b = t;
}

void shuffle_deck(Card deck[], int size)
{
    // Fisher-Yates
    for (int i = size - 1; i > 0; --i)
    {
        int j = rand() % (i + 1);
        swap_card(deck[i], deck[j]);
    }
}

bool check_face_card(Card hand[], int count)
{
    for (int i = 0; i < count; ++i)
    {
        string r = hand[i].rank;
        if (r == "J" || r == "Q" || r == "K" || r == "A")
        {
            return true;
        }
    }
    return false;
}

void show_cards(Card hand[], int count)
{
    for (int i = 0; i < count; ++i)
    {
        cout << " [" << i + 1 << "] " << hand[i].rank << " of " << hand[i].suit << endl;
    }
}

// The show_sorted_hand function shows your hand in order.
// If you give it a mapping array it will fill it so that each number you see indicates or corrsponds to the original card in your hand.
// If you don't give a mapping it just shows the sorted cards.
void show_sorted_hand(Card hand[], int count, int *mapping = nullptr)
{
    // priority order of the suits
    string suit_order[4] = {"Spades", "Clubs", "Hearts", "Diamonds"};

    // We will create a sorted view but keep previous indices

    Card sorted[52];
    int og_index[52];
    int pos = 0;

    for (int s = 0; s < 4; s++)
    {
        // collect cards of that suit with the orignal indices
        Card buffer[13];
        int buffer_index[13];
        int bcount = 0;

        for (int i = 0; i < count; i++)
        {
            if (hand[i].suit == suit_order[s])
            {
                buffer[bcount] = hand[i];
                buffer_index[bcount] = i;
                ++bcount;
            }
        }

        // sort buffer by rank
        for (int i = 0; i < bcount; i++)
        {
            int best = i;
            for (int j = i + 1; j < bcount; ++j)
            {
                if (rank_index(buffer[j].rank) < rank_index(buffer[best].rank))
                {
                    best = j;
                }
            }
            if (best != i)
            {
                swap_card(buffer[i], buffer[best]);
                int tmp = buffer_index[i];
                buffer_index[i] = buffer_index[best];
                buffer_index[best] = tmp;
            }
        }

        // append to sorted view
        for (int i = 0; i < bcount; i++)
        {
            sorted[pos] = buffer[i];
            og_index[pos] = buffer_index[i];
            ++pos;
        }
    }

    // display once
    cout << "\n--- Your hand ---\n";
    for (int i = 0; i < pos; ++i)
    {
        cout << " [" << i + 1 << "] " << sorted[i].rank << " of " << sorted[i].suit << endl;
    }
    cout << endl;

    // if mapping requested fill mapping with  mapping[display_index] = og_hand_index
    if (mapping)
    {
        for (int i = 0; i < pos; i++)
        {
            mapping[i] = og_index[i];
        }
    }
}

// remove card at index from the hand of current cards of the player of size count and then return new count
int remove_card(Card hand[], int count, int index)
{
    for (int i = index; i + 1 < count; i++)
    {
        hand[i] = hand[i + 1];
    }
    return count - 1;
}

// find index of any card of given suit in hand (returns -1 if none)
int check_suit(Card hand[], int count, const string &suit)
{
    for (int i = 0; i < count; i++)
    {
        if (hand[i].suit == suit)
        {
            return i;
        }
    }
    return -1;
}

// implementing ace on ace rule
int ace_on_ace(Card hand[], int count, const string &suit_filter, bool following_current_suite)
{
    for (int i = 0; i < count; i++)
    {
        if (hand[i].rank == "A")
        {
            continue;
        }
        if (following_current_suite)
        {
            if (hand[i].suit == suit_filter)
            {
                return i;
            }
        }
        else
        {
            return i;
        }
    }
    return -1;
}

// basic implementation of cpu logic
int cpu_logic(Card hand[], int count, const string &lead_suit, bool lead_exists, bool ace_played)
{
    // If there is a lead suit and cpu has it then he must play one of that suit
    if (lead_exists)
    {
        int index = check_suit(hand, count, lead_suit);
        if (index != -1)
        {
            // prefer highest of that suit (highest rank_index)
            int best = index;
            for (int i = index; i < count; ++i)
            {
                if (hand[i].suit == lead_suit && rank_index(hand[i].rank) > rank_index(hand[best].rank))
                {
                    best = i;
                }
            }
            // avoid playing ace if ace is already played and cpu has other legal card of that suit
            if (hand[best].rank == "A")
            {
                int alt = -1;
                for (int i = 0; i < count; i++)
                {
                    if (hand[i].suit == lead_suit && hand[i].rank != "A")
                    {
                        alt = i;
                        break;
                    }
                }
                if (alt != -1 && ace_played)
                    return alt;
            }
            return best;
        }
    }
    // If no lead suit in hand or no lead then play something else

    // prefer non ace if ace is played before
    if (ace_played)
    {
        int na = ace_on_ace(hand, count, "", false);
        if (na != -1)
            return na;
        // forced to play Ace
        for (int i = 0; i < count; i++)
        {
            if (hand[i].rank == "A")
                return i;
        }
    }
    // otherwise get rid of smallest rank
    int best = 0;
    for (int i = 1; i < count; i++)
    {
        if (rank_index(hand[i].rank) < rank_index(hand[best].rank))
            best = i;
    }
    return best;
}

// Determine trick winner given 4 played cards in order of play while considering the lead suit and the rang
// Determine trick winner correctly.
// Any rang (rang) beats any lead-suit card.
// Highest rang wins. If no rang then highest lead-suit wins.
int check_winner(Card played[], int play_order[4], const string &lead_suit, const string &rang)
{
    int winner_pos = -1;
    int best_rank = -1;

    bool rang_found = false;

    // First pass: check if ANY rang (rang) was played.
    for (int i = 0; i < 4; i++)
    {
        if (played[i].suit == rang)
            rang_found = true;
    }

    // Second pass: determine winner based on rang or lead suit
    for (int i = 0; i < 4; i++)
    {
        if (played[i].rank == "")
            continue;

        int r = rank_index(played[i].rank);

        if (rang_found)
        {
            // Only compare rang cards
            if (played[i].suit == rang && r > best_rank)
            {
                best_rank = r;
                winner_pos = i;
            }
        }
        else
        {
            // No rang played then  compare only lead-suit cards
            if (played[i].suit == lead_suit && r > best_rank)
            {
                best_rank = r;
                winner_pos = i;
            }
        }
    }

    // Convert from play-position to seat index
    return play_order[winner_pos];
}

int main()
{

    srand((unsigned)time(0));
    // build deck
    Card deck[52];
    int index = 0;
    for (int s = 0; s < 4; ++s)
    {
        for (int r = 0; r < 13; ++r)
        {
            deck[index++] = {ranks[r], suits[s]};
        }
    }

    Card hand0[13], hand1[13], hand2[13], hand3[13];
    int cnt0, cnt1, cnt2, cnt3;
    string rang;
    bool redeal;

    // Deal first 5 and allow redeal if no face has been recieved by the player
    do
    {
        redeal = false;
        shuffle_deck(deck, 52);

        // give first 5 to each
        int p = 0;
        for (int i = 0; i < 5; ++i)
        {
            hand0[i] = deck[p++];
            hand1[i] = deck[p++];
            hand2[i] = deck[p++];
            hand3[i] = deck[p++];
        }
        cnt0 = cnt1 = cnt2 = cnt3 = 5;

        cout << "--- Your first 5 cards ---" << endl;
        // show sorted but no mapping needed here
        show_sorted_hand(hand0, cnt0);

        if (!check_face_card(hand0, cnt0))
        {
            char ch;
            cout << "You have no face cards (J,Q,K,A). Redeal? (y/n): ";
            cin >> ch;
            if (ch == 'y' || ch == 'Y')
            {
                redeal = true;
                continue;
            }
        }

        // choose rang
        do
        {
            cout << "Choose rang suit (Hearts / Diamonds / Clubs / Spades): ";
            cin >> rang;

            if (rang != "Hearts" && rang != "hearts" && rang != "diamonds" && rang != "clubs" && rang != "spades" && rang != "Diamonds" &&
                rang != "Clubs" && rang != "Spades")
            {
                cout << "Invalid suit! Please enter again.\n";
            }

        } while (rang != "Hearts" && rang != "hearts" && rang != "diamonds" && rang != "clubs" && rang != "spades" && rang != "Diamonds" &&
                 rang != "Clubs" && rang != "Spades");

        if (rang == "hearts" || rang == "Hearts")
            rang = "Hearts";
        else if (rang == "diamonds" || rang == "Diamonds")
            rang = "Diamonds";
        else if (rang == "clubs" || rang == "Clubs")
            rang = "Clubs";
        else if (rang == "spades" || rang == "Spades")
            rang = "Spades";

        // deal remaining 8 cards to each (continue from where we left off)
        int deal_pos = 20; // 5 * 4
        for (int i = 5; i < 13; ++i)
        {
            hand0[i] = deck[deal_pos++];
            ++cnt0;
            hand1[i] = deck[deal_pos++];
            ++cnt1;
            hand2[i] = deck[deal_pos++];
            ++cnt2;
            hand3[i] = deck[deal_pos++];
            ++cnt3;
        }
    } while (redeal);

    cout << "rang is: " << rang << endl;

    // show full sorted hand (no mapping)
    show_sorted_hand(hand0, cnt0);

    // Play 13 tricks. We'll let player 0 (you) start first.
    int won_tricks[4] = {0, 0, 0, 0};
    int leader = 0; // seat index 0 = your own , 1 = cpu1, 2 = cpu2, 3 = cpu3

    // To maintain hands compactly we keep cnts and arrays; after removing we shift left.
    for (int t = 1; t <= 13; ++t)
    {
        cout << "===== Trick " << t << " =====" << endl;
        Card played_cards[4];
        for (int i = 0; i < 4; ++i)
        {
            played_cards[i] = {"", ""};
        }

        int play_order[4]; // maps play position to seat index
        string lead_suit = "";
        bool ace_played_in_trick = false;

        // Each player in order (leader, leader+1 mod4, ...)
        for (int seat_off_set = 0; seat_off_set < 4; ++seat_off_set)
        {
            int seat = (leader + seat_off_set) % 4;
            play_order[seat_off_set] = seat;

            Card *hand;
            int *cnt;
            if (seat == 0)
            {
                hand = hand0;
                cnt = &cnt0;
            }
            else if (seat == 1)
            {
                hand = hand1;
                cnt = &cnt1;
            }
            else if (seat == 2)
            {
                hand = hand2;
                cnt = &cnt2;
            }
            else
            {
                hand = hand3;
                cnt = &cnt3;
            }

            // Determine legal play:
            bool lead_exists = (lead_suit != "");
            int must_follow_index = -1;
            if (lead_exists)
                must_follow_index = check_suit(hand, *cnt, lead_suit);

            int chosen_index = -1;

            if (seat == 0)
            {
                // Human player
                // Provide mapping so displayed numbers map back to actual hand indices
                int display_map[13];
                show_sorted_hand(hand0, cnt0, display_map);

                // indicate lead_suit
                if (!lead_exists)
                {
                    cout << "You lead this trick." << endl;
                }
                else
                {
                    cout << "Lead suit is: " << lead_suit << " (you must follow if you have it)." << endl;
                }

                bool valid = false;
                while (!valid)
                {
                    int pick_display; // index user sees (1..cnt)
                    cout << "Choose card number to play: ";
                    cin >> pick_display;
                    if (cin.fail()) // detects string input which we dont want
                    {
                        cout << "Invalid input! Please enter a number." << endl;
                        cin.clear();            // reset fail state
                        cin.ignore(1000, '\n'); // clear buffer
                        continue;
                    }
                    if (pick_display < 1 || pick_display > *cnt)
                    {
                        cout << "Invalid index. Try again." << endl;
                        continue;
                    }
                    int pick_display0 = pick_display - 1;         // convert to 0-based display index
                    int pick_actual = display_map[pick_display0]; // translate to actual hand[] index

                    // check must-follow rule
                    if (must_follow_index != -1 && hand[pick_actual].suit != lead_suit)
                    {
                        cout << "You must follow the lead suit (" << lead_suit << "). Choose a " << lead_suit << " card." << endl;
                        continue;
                    }

                    // check ace-on-ace rule which mean that if an ace has already been played in this trick, disallow playing another Ace unless player has no other legal card.

                    bool trying_ace = (hand[pick_actual].rank == "A");
                    if (ace_played_in_trick && trying_ace)
                    {
                        // check if player has other legal card

                        bool has_alternative = false;
                        for (int i = 0; i < *cnt; ++i)
                        {
                            if (i == pick_actual)
                                continue;
                            if (must_follow_index != -1)
                            {
                                if (hand[i].suit == lead_suit && hand[i].rank != "A")
                                {
                                    has_alternative = true;
                                    break;
                                }
                                if (hand[i].suit == lead_suit && hand[i].rank == "A")
                                    continue;
                            }
                            else
                            {
                                if (hand[i].rank != "A")
                                {
                                    has_alternative = true;
                                    break;
                                }
                            }
                        }
                        if (has_alternative)
                        {
                            cout << "Ace-on-Ace not allowed while you have another legal card. Choose another card.\n";
                            continue;
                        }
                        // else forced to play Ace as no alternative
                    }

                    // valid move
                    chosen_index = pick_actual;
                    valid = true;
                }
            }
            else
            {
                // CPU chooses
                chosen_index = cpu_logic(hand, *cnt, lead_suit, lead_exists, ace_played_in_trick);
            }

            // play the chosen card
            Card played = hand[chosen_index];
            // remove from hand
            *cnt = remove_card(hand, *cnt, chosen_index);

            // set lead_suit if this is the first play
            if (!lead_exists)
                lead_suit = played.suit;

            // record if an Ace is played
            if (played.rank == "A")
            {
                ace_played_in_trick = true;
            }

            // store in played_cards in position seat_off_set (play order)
            played_cards[seat_off_set] = played;

            // show play
            if (seat == 0)
                cout << "You played: " << played.rank << " of " << played.suit << endl;
            else
                cout << "Player " << seat + 1 << " played: " << played.rank << " of " << played.suit << endl;
        } // end 4 players playing

        // Decide winner
        int winner_seat = check_winner(played_cards, play_order, lead_suit, rang);
        cout << "Trick winner: Player " << (winner_seat + 1) << " (will lead next trick)" << endl;
        ++won_tricks[winner_seat];
        leader = winner_seat;
    } // end tricks

    cout << "===== FINAL SCORES =====" << endl;

    for (int i = 0; i < 4; ++i)
    {
        cout << "Player " << i + 1 << " won " << won_tricks[i] << " tricks." << endl;
    }

    int your_team = won_tricks[0] + won_tricks[2];
    int team_against = won_tricks[1] + won_tricks[3];
    cout << "Your team (You + Player 3): " << your_team << "  Opponent team: " << team_against << endl;
    if (your_team > team_against)

        cout << "Your team wins!" << endl;

    else if (your_team < team_against)
        cout << "Opponents win!" << endl;
    else
        cout << "It's a tie!" << endl;

    return 0;
}