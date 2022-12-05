#include <iostream> 
#include <vector> 
#include <algorithm> 
#include <ctime> 

class Card{
public:
	enum rank {
		ACE = 1, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN,
		JACK, QUEEN, KING
	};
	enum suit { CLUBS, DIAMONDS, HEARTS, SPADES };
	friend std::ostream& operator<<(std::ostream& os, const Card& aCard);
	Card(rank r = ACE, suit s = SPADES, bool ifu = true);
	int32_t Get_Value() const;
	void Flip();
private:
	rank m_Rank;
	suit m_Suit;
	bool m_IsFaceUp;
};

Card::Card(rank r, suit s, bool ifu) : m_Rank(r), m_Suit(s), m_IsFaceUp(ifu){}

int32_t Card::Get_Value() const{
	int32_t value = 0;
	if (m_IsFaceUp)
	{
		value = m_Rank;
		if (value > 10) {
			value = 10;
		}
	}
	return value;
}

void Card::Flip(){
	m_IsFaceUp = !(m_IsFaceUp);
}

class Hand{
protected:
	std::vector<Card*> m_Cards;
public:
	Hand();
	virtual ~Hand();
	void Add(Card* pCard);
	void Clear(); int32_t Get_Total() const;
};

Hand::Hand(){
	m_Cards.reserve(7);
}

Hand::~Hand(){
	Clear();
}

void Hand::Add(Card* pCard){
	m_Cards.push_back(pCard);
}

void Hand::Clear(){
	std::vector<Card*>::iterator iter = m_Cards.begin();
	for (iter = m_Cards.begin(); iter != m_Cards.end(); ++iter)
	{
		delete* iter;
		*iter = 0;
	}
	m_Cards.clear();
}

int32_t Hand::Get_Total() const{
	if (m_Cards.empty())
	{
		return 0;
	}

	if (m_Cards[0]->Get_Value() == 0)
	{
		return 0;
	}

	int32_t total = 0;
	std::vector<Card*>::const_iterator iter;
	for (iter = m_Cards.begin(); iter != m_Cards.end(); ++iter)
	{
		total += (*iter)->Get_Value();
	}

	bool containsAce = false;
	for (iter = m_Cards.begin(); iter != m_Cards.end(); ++iter)
	{
		if ((*iter)->Get_Value() == Card::ACE)
		{
			containsAce = true;
		}
	}

	if (containsAce && total <= 11)
	{
		total += 10;
	}
	return total;
}


class GenericPlayer : public Hand{
private:
	friend std::ostream& operator<<(std::ostream& os, const GenericPlayer& aGenericPlayer);
protected:
	std::string m_Name;
public:
	GenericPlayer(const std::string& name = "");
	virtual ~GenericPlayer();
	virtual bool IsHitting() const = 0;
	bool IsBusted() const;
	void Bust() const;
};

GenericPlayer::GenericPlayer(const std::string& name) :	m_Name(name){}
GenericPlayer::~GenericPlayer(){}

bool GenericPlayer::IsBusted() const{
	return (Get_Total() > 21);
}

void GenericPlayer::Bust() const{
	std::cout << "\033[1;36m" << m_Name << "\033[0m\ " << "\033[1;31m  busts.\033[0m\n";
}


class Player : public GenericPlayer{
public:
	Player(const std::string& name = "");
	virtual ~Player();
	virtual bool IsHitting() const;
	void Win() const;
	void Lose() const;
	void Push() const;
};
Player::Player(const std::string& name) :GenericPlayer(name){}

Player::~Player(){}

bool Player::IsHitting() const
{
	std::cout << m_Name << ", do you want a hit? (Y/N): ";
	char response;
	std::cin >> response;
	return (response == 'y' || response == 'Y');
}

void Player::Win() const
{
	std::cout << "\033[1;36m" << m_Name << "\033[0m\ " << "\033[1;32m  wins.\033[0m\n";
}

void Player::Lose() const{
	std::cout << "\033[1;36m" << m_Name << "\033[0m\ " << "\033[1;31m  loses.\033[0m\n";
}

void Player::Push() const{
	std::cout << "\033[1;36m" << m_Name << "\033[0m\ " << "\033[1;30m  pushes.\033[0m\n";
}

class Dealer  : public GenericPlayer{
public:
	Dealer (const std::string& name = "Dealer ");
	virtual ~Dealer ();
	virtual bool IsHitting() const;
	void FlipFirstCard();
};

Dealer ::Dealer (const std::string& name) : GenericPlayer(name){}

Dealer ::~Dealer (){}

bool Dealer ::IsHitting() const{
	return (Get_Total() <= 16);
}

void Dealer ::FlipFirstCard(){
	if (!(m_Cards.empty()))
	{
		m_Cards[0]->Flip();
	}
	else
	{
		std::cout << "No card to flip!\n";
	}
}

class Deck : public Hand{
public:
	Deck();
	virtual ~Deck();
	void Populate();
	void Shuffle();
	void Deal(Hand& aHand);
	void AdditionalCards(GenericPlayer& aGenericPlayer);
};

Deck::Deck(){
	m_Cards.reserve(52);
	Populate();
}
Deck::~Deck(){}

void Deck::Populate(){
	Clear();
	for (int32_t s = Card::CLUBS; s <= Card::SPADES; ++s)
	{
		for (int32_t r = Card::ACE; r <= Card::KING; ++r)
		{
			Add(new Card(static_cast<Card::rank>(r),
				static_cast<Card::suit>(s)));
		}
	}
}

void Deck::Shuffle(){
	random_shuffle(m_Cards.begin(), m_Cards.end());	
}

void Deck::Deal(Hand& aHand){
	if (!m_Cards.empty())
	{
		aHand.Add(m_Cards.back());
		m_Cards.pop_back();
	}
	else
	{
		std::cout << "Out of cards. Unable to deal.";
	}
}

void Deck::AdditionalCards(GenericPlayer& aGenericPlayer){
	std::cout << std::endl;
	while (!(aGenericPlayer.IsBusted()) && aGenericPlayer.IsHitting())
	{
		Deal(aGenericPlayer);
		std::cout << aGenericPlayer << std::endl;
		if (aGenericPlayer.IsBusted())
		{
			aGenericPlayer.Bust();
		}
	}
}

class Game{
private:
	Deck m_Deck;
	Dealer  m_Dealer;
	std::vector<Player> m_Players;
public:
	Game(const std::vector<std::string>& names);
	~Game();
	void Play();
};

Game::Game(const std::vector<std::string>& names){
	std::vector<std::string>::const_iterator pName;
	for (pName = names.begin(); pName != names.end(); ++pName){
		m_Players.push_back(Player(*pName));
	}
	srand(static_cast<unsigned int>(time(0)));
	m_Deck.Populate();
	m_Deck.Shuffle();
}
Game::~Game(){}

void Game::Play(){
	std::vector<Player>::iterator pPlayer;

	for (int32_t i = 0; i < 2; ++i)
	{
		for (pPlayer = m_Players.begin(); pPlayer != m_Players.end();
			++pPlayer)
		{
			m_Deck.Deal(*pPlayer);
		}
		m_Deck.Deal(m_Dealer );
	}
	m_Dealer .FlipFirstCard();

	for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer)
	{
		std::cout << *pPlayer << std::endl;
	}
	std::cout << m_Dealer  << std::endl;

	for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer)
	{
		m_Deck.AdditionalCards(*pPlayer);
	}
	m_Dealer .FlipFirstCard();
	std::cout << std::endl << m_Dealer ;
	m_Deck.AdditionalCards(m_Dealer );

	if (m_Dealer .IsBusted()){
		for (pPlayer = m_Players.begin(); pPlayer != m_Players.end();
			++pPlayer)
		{
			if (!(pPlayer->IsBusted()))
			{
				pPlayer->Win();
			}
		}
	}

	else{
		for (pPlayer = m_Players.begin(); pPlayer != m_Players.end();
			++pPlayer)
		{
			if (!(pPlayer->IsBusted())){
				if (pPlayer->Get_Total() > m_Dealer .Get_Total())
				{
					pPlayer->Win();
				}
				else if (pPlayer->Get_Total() < m_Dealer .Get_Total())
				{
					pPlayer->Lose();
				}
				else
				{
					pPlayer->Push();
				}
			}
		}
	}

	for (pPlayer = m_Players.begin(); pPlayer != m_Players.end(); ++pPlayer){
		pPlayer->Clear();
	}
	m_Dealer .Clear();
}

std::ostream& operator<<(std::ostream& os, const Card& aCard){
	const std::string RANKS[] = { "0", "A", "2", "3", "4", "5", "6", "7", "8", "9",
	"10", "J", "Q", "K" };
	const std::string SUITS[] = { "c", "d", "h", "s" };
	if (aCard.m_IsFaceUp){
		os << RANKS[aCard.m_Rank] << SUITS[aCard.m_Suit];
	}
	else{
		os << "XX";
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const GenericPlayer& aGenericPlayer){
	os << aGenericPlayer.m_Name << ":\t";
	std::vector<Card*>::const_iterator pCard;
	if (!aGenericPlayer.m_Cards.empty()){
		for (pCard = aGenericPlayer.m_Cards.begin();
			pCard != aGenericPlayer.m_Cards.end();
			++pCard)
		{
			os << *(*pCard) << "\t";
		}
		if (aGenericPlayer.Get_Total() != 0)
		{
			std::cout << "(" << aGenericPlayer.Get_Total() << ")";
		}
	}
	else{
		os << "<empty>";
	}
	return os;
}

auto main()->int32_t{
	std::cout << "\t\t\033[1;35m Welcome to Blackjack!\033[0m\n\n";
	int32_t numPlayers = 0;

	while (numPlayers < 1 || numPlayers > 7){
		std::cout << "\033[1;31mHow many players? (1 - 7):\033[0m ";
		std::cin >> numPlayers;
	}

	std::vector<std::string> names;
	std::string name;

	for (int32_t i = 0; i < numPlayers; ++i){
		std::cout << "Enter player name: ";
		std::cin >> name;
		names.push_back(name);
	}

	std::cout << std::endl;
	Game aGame(names);
	char again = 'y';

	while (again != 'n' && again != 'N'){
		aGame.Play();
		std::cout << "\n\033[1;34mDo you want to play again? (Y/N):\033[0m ";
		std::cin >> again;
	}

	return 0;
}



















