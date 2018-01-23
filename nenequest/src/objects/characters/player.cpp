#include <player.hpp>
#include <world.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
/**
    TODO (not updated) :
    - Placer l'arme correctement
    - Animation de l'attaque (rotation du sprite du Weapon)
    - Empecher de sortir de la fenere / allez trop haut-bas => Fait dans game.cpp <= and fixed back here
    - Detection avec les ennemis => Fait dans game.cpp 
    - Frame immortalite lorsqu'on perd de la vie => Fait dans game.cpp mais c'est mieux ici donc fait le +(animation clignotante dans l'ideal)
    - Saut (immortalite temporaire) => Fait dans game.cpp <= need to be here
**/

using namespace sf;
using namespace std;

//Weapon placement - edited in Player constructor
int weaponXOffsetP1 = 233;
int weaponYOffsetP1 = 190;
int weaponXOffsetVariationP1 = 2;
int weaponYOffsetVariationP1 = 2;
int weaponXOffsetP2 = 235;
int weaponYOffsetP2 = 170;
int weaponXOffsetVariationP2 = 0;
int weaponYOffsetVariationP2 = 3;

//Z Hitbox sizes
float z_offset_x = 0.16;
float z_offset_y = 0.8;
float z_width = 0.5;
float z_height = 0.2;

//Sprite animation tiles
FloatRect fwalk1normal = FloatRect(0, 0, 0.5f, 0.333333f);
FloatRect fwalk2normal = FloatRect(0, 0.333333f, 0.5f, 0.333333f);
FloatRect fmoddifierAttack = FloatRect(0.5f, 0, 0, 0);
IntRect walk1normal;
IntRect walk2normal;
IntRect moddifierAttack;

Player::Player(Weapon* w, Vector2f position, bool secondPlayer) { // 150,170

    this->weapon = w;
    if (!secondPlayer) {
        this->life = new LifeBar(PLAYER_HP, Vector2f(300,100), PlayerID::PLAYER1);
        this->texture.loadFromFile("img/player1.png");
		this->weaponXOffset = weaponXOffsetP1;
		this->weaponYOffset = weaponYOffsetP1;
		this->weaponXOffsetVariation = weaponXOffsetVariationP1;
		this->weaponYOffsetVariation = weaponYOffsetVariationP1;
    } else {
        this->life = new LifeBar(PLAYER_HP, Vector2f(800,100), PlayerID::PLAYER2);
		this->texture.loadFromFile("img/player2.png");
		this->weaponXOffset = weaponXOffsetP2;
		this->weaponYOffset = weaponYOffsetP2;
		this->weaponXOffsetVariation = weaponXOffsetVariationP2;
		this->weaponYOffsetVariation = weaponYOffsetVariationP2;
	}

	this->sprite.setTexture(this->texture);
	this->sprite.setTextureRect(IntRect(0, 0, this->texture.getSize().x/2, this->texture.getSize().y/3));

	//Hitboxes + debug coloring
	sf::RectangleShape* h = new sf::RectangleShape();
	h->setFillColor(sf::Color(255, 0, 0, 128));	
	sf::RectangleShape* zh = new sf::RectangleShape();
	zh->setFillColor(sf::Color(0, 0, 255, 128));
	
	hitboxes.push_back(h);
	zHitboxes.push_back(zh);

	this->hitboxes.at(0)->setPosition(position);
	this->hitboxes.at(0)->setSize(Vector2f(this->sprite.getLocalBounds().width, this->sprite.getLocalBounds().height));
	this->zHitboxes.at(0)->setPosition(position + sf::Vector2f(this->sprite.getLocalBounds().width * z_offset_x, this->sprite.getLocalBounds().height * z_offset_y));
	this->zHitboxes.at(0)->setSize(Vector2f(this->sprite.getLocalBounds().width * z_width, this->sprite.getLocalBounds().height * z_height));

    updateAutoSpritePosition();

	// Weapon placement
	this->weapon->setPosition(position.x + this->weaponXOffset, position.y + this->weaponYOffset);

	//Generate animations rects with sizes
	vector<FloatRect> fanimationRects = { fwalk1normal, fwalk2normal, fmoddifierAttack };
	vector<IntRect> animationRects;
	int sizeX = texture.getSize().x;
	int sizeY = texture.getSize().y;
	for each (FloatRect rect in fanimationRects)
	{
		IntRect r;
		r.left = rect.left * sizeX;
		r.width = rect.width * sizeX;
		r.top = rect.top * sizeY;
		r.height = rect.height * sizeY;
		animationRects.push_back(r);
	}

	walk1normal = animationRects.at(0);
	walk2normal = animationRects.at(1);
	moddifierAttack = animationRects.at(2);
}

Player::~Player(){
    delete this->weapon;
}

Weapon* Player::getWeapon(){
    return this->weapon;
}

void Player::attack(){
    this->is_attacking = true;
    update_animation();
    this->is_attacking = false;
}

void Player::equip(Weapon* w){
    this->weapon = w;
    this->weapon->setPosition(this->getPosition().x + this->weaponXOffset, this->getPosition().y + this->weaponYOffset);
}

void Player::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(*this->weapon, states);
    target.draw(*this->life, states);
	Entity::draw(target, states);
	for each (Entity* var in arrows)
	{
		target.draw(*var, states);
	}
}

IntRect IntRectAddition(IntRect a, IntRect b) {
	IntRect result = IntRect(a.left + b.left, a.top + b.top, a.width + b.width, a.height + b.height);
	return result;
}

void Player::update_animation(){
    this->animation_state = !this->animation_state;
	int weaponXDecal = this->weaponXOffset;
	int weaponYDecal = this->weaponYOffset;
	IntRect result;
    
	if (this->animation_state) { // jambes croisees
		result = walk2normal;
		weaponXDecal += this->weaponXOffsetVariation;
		weaponYDecal += this->weaponYOffsetVariation;
	}
	else {
		result = walk1normal;
	}
	
	if (this->isJumping())
        return;	// result = IntRectAddition(result, moddifierJump);
	
	if (is_attacking)
		result = IntRectAddition(result, moddifierAttack);

    sprite.setTextureRect(result);
	this->weapon->setPosition(this->getPosition().x + weaponXDecal, this->getPosition().y + weaponYDecal);
}


// Called every frame by manageMovements->fixPosition->move
void Player::move(Vector2f g_speed){
	float elapsedTime = World::getElapsedTime();
    if (clock.getElapsedTime().asSeconds() > this->ANIMATION_DELAY) {
        update_animation();
        clock.restart();
    }

	Entity::move(g_speed);
	//sf::Vector2f v = sf::Vector2f(g_speed.x * elapsedTime, g_speed.y*elapsedTime);
	this->getWeapon()->move(g_speed);
}

//Deprecated : use Entity->get/alterHealth()
//LifeBar* Player::getLife(){
//    return this->life;
//}

bool Player::isJumping(){
     return this->is_jumping;
}

void Player::toggleJump(){
    this->is_jumping = !this->is_jumping;
}

void Player::setPosition(float x, float y){
	Entity::setPosition(x, y);
    this->weapon->setPosition(this->getPosition().x + weaponXOffset, this->getPosition().y + weaponYOffset);
}

void Player::fireArrow(){
    arrows.push_back(new Arrow(Vector2f(this->getPosition().x + weaponXOffset, this->getPosition().y + weaponYOffset), this->getDepth()));
	setShootingState(true);
}

void Player::update(float elapsedTime){
	this->cleanArrows(elapsedTime);
	this->manageMovements();
}

void Player::manageMovements() {
	Vector2f finalMovement = Vector2f(0, 0);
	if (this->moving_up)
		finalMovement.y -= PLAYER_SPEED;
	if (this->moving_down)
		finalMovement.y += PLAYER_SPEED;
	if (this->moving_left)
		finalMovement.x -= PLAYER_SPEED;
	if (this->moving_right)
		finalMovement.x += PLAYER_SPEED;

	finalMovement = this->fixMovements(finalMovement);
	sf::Vector2f fx = sf::Vector2f(finalMovement.x, 0);
	sf::Vector2f fy = sf::Vector2f(0, finalMovement.y);
	if (World::testCollidingEntitiesOnZAxis(this, fx).size() == 0)
		this->move(fx);
	if (World::testCollidingEntitiesOnZAxis(this, fy).size() == 0)
		this->move(fy);

	this->fixPosition();
}

//TODO : remove hardcoded value for cute ones
sf::Vector2f Player::fixMovements(sf::Vector2f movement) {
	Vector2f result = Vector2f(movement.x * World::getElapsedTime(), movement.y * World::getElapsedTime());
	if (this->hitboxes.at(0)->getGlobalBounds().left + result.x < 0)
		result.x = 0;
		//result.x = -this->hitbox.getGlobalBounds().left;

	if (this->hitboxes.at(0)->getGlobalBounds().left + this->hitboxes.at(0)->getGlobalBounds().width + result.x > 1920 /*Gameroom size (right)*/)
		result.x = 0;
		//result.x = 1920 - this->hitbox.getGlobalBounds().left - this->hitbox.getGlobalBounds().width;

	//TODO
	if (this->hitboxes.at(0)->getGlobalBounds().top + result.y < 200 /*Gameroom size (top)*/)
		result.y = 0;
		//result.y = 500 - this->hitbox.getGlobalBounds().top;

	if (this->hitboxes.at(0)->getGlobalBounds().top + this->hitboxes.at(0)->getGlobalBounds().height + result.y > 1080 /*Gameroom size (bottom)*/)
		result.y = 0;
		//result.y = 1080 - this->hitbox.getGlobalBounds().top - this->hitbox.getGlobalBounds().height;
	
	
	return result;
}

//TODO : remove hardcoded value for cute ones
void Player::fixPosition() {
	sf::FloatRect bounds = this->hitboxes.at(0)->getGlobalBounds();
	sf::Vector2f v = Vector2f(0, 0);
	if (bounds.left < 0)
		v.x = -bounds.left;

	if (bounds.left + bounds.width > 1920)
		v.x = (1920 - bounds.left - bounds.width);

	if (bounds.top < 200)
		v.y = -1 * (bounds.top - 200);

	if (bounds.top + bounds.height > 1080)
		v.y = (1080 - bounds.top - bounds.height);

	this->move(v);

}

void Player::cleanArrows(float elapsedTime) {
	for (unsigned int i = 0; i< this->arrows.size(); i++) {
		this->arrows.at(i)->update(elapsedTime);
		if (this->arrows.at(i)->isDead()) {
			delete(this->arrows.at(i));
			this->arrows.erase(this->arrows.begin() + i);
		}
	}
}

void Player::setLastDroppedItem(ItemWeapon* item){
    last_dropped_item = item;
}

ItemWeapon* Player::getLastDroppedItem(){
    return last_dropped_item;
}

vector<Arrow*> Player::getArrows(){
    return arrows;
}

bool Player::isShooting() {
	return is_shooting;
}

void Player::setShootingState(bool state) {
	is_shooting = state;
}