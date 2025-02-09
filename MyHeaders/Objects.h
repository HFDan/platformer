#ifndef OBJECTS
#define OBJECTS
#include "ExtendedRaylib.h"
#include "Player.h"
#include <map>
#include <utility> /// for pair
#include <fstream> /// for accessing stored levels
#include <stdio.h> /// for replacing (meaning deleting) current level
extern Texture2D ASSET_BLOCKS;
extern char doing[21];
extern std::map<int,int> UID_pairing;
class Object;
class Block;
class Start;
class Finish;
class MapObj;

class Object
{
    /// D,U,L,R = directions (down, ...), p = player, b = block
    /*#define UbDp (1.0f*(hitbox.y+y)  -  (myPlayer.getPrevHitbox().y+myPlayer.hitbox.height))
    #define LbRp (1.0f*(x+hitbox.x)  -  (myPlayer.getPrevHitbox().x+myPlayer.hitbox.width))
    #define UpDb (1.0f*(myPlayer.getPrevHitbox().y)  -  (y+hitbox.y+hitbox.height))
    #define LpRb (1.0f*(myPlayer.getPrevHitbox().x)  -  (x+hitbox.x+hitbox.width))
*/
    public:
    bool canWJ=false;
    Texture2D image;
    int imageX;
    int UID;
    int page;
    Rectangle hitbox;
    Object(){}
    Directions getDir(int x,int y)
    {
        Directions rez;
        rez.up=y+hitbox.y;
        rez.down=y+hitbox.y+hitbox.height;
        rez.left=x+hitbox.x;
        rez.right=x+hitbox.x+hitbox.width;
        return rez;
    }
    Rectangle getHitbox(int x,int y)
    {
        return {x+hitbox.x,y+hitbox.y,hitbox.width,hitbox.height};
    }
    Object(int UID,int page,Texture2D image,int imageX,Rectangle hitbox):page(page)
    {
        this->UID=UID;
        this->image=image;
        this->imageX=imageX;
        this->hitbox=hitbox;
    }
    void virtual draw(int x,int y,int transparency=255)
    {
        Color white=WHITE;
        white.a=transparency;
        Rectangle imageBoundaries={(float)imageX,0,getImageSize().x,getImageSize().y};
        Vector2 pos={(float)x,(float)y};
        DrawTextureRec(image,imageBoundaries,pos,white);
        if(!hideHitbox)
        {
            Rectangle objectRect={x+hitbox.x,y+hitbox.y,hitbox.width,hitbox.height};
            DrawRectangleLinesEx(objectRect,1,WHITE);
        }

    }
    void movePlayer(char const c[10],int x,int y); /// Had to declare elsewhere
    void virtual collisionEffect(int x,int y){}
    void virtual specialEffect(){}
    Vector2 const virtual getImageSize()
    {
        return {32,32};
    }
    bool collision(int x,int y,Rectangle entity)
    {
        if(CheckCollisionRecs(entity,this->getHitbox(x,y)))
            return true;
        return false;
    }

};
extern Object **AllObjects;
class Block : public Object
{
    public:
    Block(){}
    Block(int UID,int page,Texture2D image,int imageX,Rectangle hitbox):Object(UID,page,image,imageX,hitbox){canWJ=true;}
    void collisionEffect(int x,int y)
    {
        Directions playerDir=myPlayer.getOldDir();
        Directions objDir=getDir(x,y);
        int up   =  objDir.up - playerDir.down;
        int down =  playerDir.up - objDir.down;
        int left =  objDir.left - playerDir.right;
        int right = playerDir.left - objDir.right;
        if(up>=0)
            movePlayer("up",x,y);else
        if(down>=0)
            movePlayer("down",x,y);
        if(left>=0)
            movePlayer("left",x,y);else
        if(right>=0)
            movePlayer("right",x,y);
    }
};
Block Block1,Block2,Block3,Block4,Block5;
class Start : public Object
{
    public:
    int x=0,y=0;
    bool exists=false;
    Start(){}
    Start(int UID,int page,Texture2D image,int imageX,Rectangle hitbox):Object(UID,page,image,imageX,hitbox){}
    void specialEffect()
    {
        myPlayer.xCoord=x+16-myPlayer.hitbox.x-myPlayer.hitbox.width/2;
        myPlayer.yCoord=y+16-myPlayer.hitbox.y-myPlayer.hitbox.height/2;
    }
    Vector2 const getImageSize()
    {
        return {32,64};
    }
    bool collision(Rectangle entity)
    {
        return Object::collision(x,y,entity);
    }
    void draw(int transparency=255)
    {
        return Object::draw(x,y,transparency);
    }

}myStart;
void Player::reset()
{
    myStart.specialEffect();
    xVelocity=yVelocity=0;
    xFacing=1;
    XDirection=0;
    isdashing=false;
    presume();
}
class Finish : public Object
{
    public:
    int x=100,y=0;
    bool exists=false;
    bool won=false;
    Finish(){}
    Finish(int UID,int page,Texture2D image,int imageX,Rectangle hitbox):Object(UID,page,image,imageX,hitbox){}
    void draw(int transparency)
    {
        Object::draw(x,y,transparency);
    }
    bool collision(Rectangle entity)
    {
        return Object::collision(x,y,entity);
    }
    void collisionEffect()
    {
        won=true;
    }
}myFinish;

class MapObj
{
public:
    std::map <std::pair<int,int>,int> currentMap;
    void saveMap(std::string fileName)
    {
        /// TO DO: make file maybe if aint existin'
        remove(fileName.c_str());
        std::ofstream fout(fileName);
        fout<<"start "<<myStart.x<<' '<<myStart.y<<'\n';
        fout<<"finish "<<myFinish.x<<' '<<myFinish.y<<'\n';
        for(std::map <std::pair<int,int>,int>::iterator it=currentMap.begin(); it!=currentMap.end(); it++)
            fout<<(it->first).first<<' '<<(it->first).second<<' '<<AllObjects[it->second]->UID<<'\n';
        fout.close();
    }
    void loadMap(std::string fileName)
    {
        currentMap.clear();
        std::ifstream fin(fileName);
        if(!fin)
        {std::cout<<"Error";return;}

        char c[10];
        fin>>c;
        if(strcmp(c,"start"))
        {
            std::cout<<"\n\nError, no start provided\n\n";
            strcpy(doing,"Exiting");return;
        }
        fin>>myStart.x>>myStart.y;
        fin>>c;
        if(strcmp(c,"finish"))
        {
            std::cout<<"\n\nError, no finish provided\n\n";
            strcpy(doing,"Exiting");return;
        }
        fin>>myFinish.x>>myFinish.y;

        myStart.specialEffect(); /// in this case sets player position

        std::pair<int,int> coord;
        int UID;
        while(fin>>coord.first)
        {
            fin>>coord.second>>UID;
            if(UID==myStart.UID || UID==myFinish.UID)
            {
                std::cout<<"\n\nError, multiple starts or finishes provided.\n\n";
                strcpy(doing,"Exiting");return;
            }
            currentMap[coord]=UID_pairing[UID];
        }
        fin.close();
    }
    void drawMap(int transparency)
    {
        myStart.draw(transparency);
        myFinish.draw(transparency);
        for(std::map <std::pair<int,int>,int>::iterator it=currentMap.begin(); it!=currentMap.end(); it++)
            AllObjects[(it->second)]->draw((it->first).first,(it->first).second,transparency);
    }
    void checkAllCollisions()
    {
        if(myFinish.collision(myPlayer.getHitbox()))myFinish.collisionEffect();
        for(std::map <std::pair<int,int>,int>::iterator it=currentMap.begin(); it!=currentMap.end(); it++)
        {
            Object *obj=AllObjects[it->second];
            if(obj->collision((it->first).first , (it->first).second , myPlayer.getHitbox()))
                obj->collisionEffect((it->first).first , (it->first).second);
            if(obj->canWJ)
                myPlayer.tryWallJump(obj->getDir(it->first.first,it->first.second));

        }

        myPlayer.newMovement();
    }
    bool checkAllCollisionsE(Rectangle entity)
    {
        if(myFinish.collision(entity)) return true;
        if(myStart.collision(entity))  return true;
        for(std::map <std::pair<int,int>,int>::iterator it=currentMap.begin(); it!=currentMap.end(); it++)
            if(AllObjects[it->second]->collision((it->first).first , (it->first).second , entity))
                return true;
        return false;
    }
    bool checkAllCollisionsMouse()
    {
        return checkAllCollisionsE({(float)GetMouseX(),(float)GetMouseY(),0,0});
    }
    std::pair<int,int> getCollisionE(Rectangle entity)
    {
        for(std::map <std::pair<int,int>,int>::iterator it=currentMap.begin(); it!=currentMap.end(); it++)
            if(AllObjects[it->second]->collision((it->first).first , (it->first).second , entity))
                return {(it->first).first , (it->first).second};
        return {-999999,-999999};
    }
    std::pair<int,int> getCollisionMouse()
    {
        return getCollisionE({(float)GetMouseX(),(float)GetMouseY(),0,0});
    }
    void deletePair(std::pair<int,int> coord)
    {
        currentMap.erase(currentMap.find(coord));
    }
    void deleteClick(Vector2 pos)
    {
        Rectangle entity={pos.x,pos.y,1,1};
        for(std::map <std::pair<int,int>,int>::iterator it=currentMap.begin(); it!=currentMap.end(); it++)
            if(AllObjects[it->second]->collision((it->first).first , (it->first).second , entity))
            {
                currentMap.erase(it);
                return;
            }
    }
} myMap;
void Object::movePlayer(char const c[10],int x,int y)
{
    Directions playerDir=myPlayer.getOldDir();
    Directions objDir=getDir(x,y);
    float moveUp    = objDir.up - myPlayer.hitbox.y - myPlayer.hitbox.height;
    float moveDown  = objDir.down - myPlayer.hitbox.y;
    float moveLeft  = objDir.left - myPlayer.hitbox.x - myPlayer.hitbox.width;
    float moveRight = objDir.right - myPlayer.hitbox.x +1e-4; /// error correction
    if(!strcmp(c,"up"))
    {

        myPlayer.yCoord=moveUp;
        myPlayer.yVelocity=0;
        myPlayer.isGrounded=true;
    }

    if(!strcmp(c,"left"))
    {
        if(objDir.up-playerDir.down>-10
        && !myMap.checkAllCollisionsE({myPlayer.xCoord,moveUp,myPlayer.getHitbox().width,myPlayer.getHitbox().height}))
        {
            return movePlayer("up",x,y);
        }

        myPlayer.xCoord=moveLeft;
        myPlayer.xVelocity=0;

    }
    if(!strcmp(c,"right"))
    {
        if(objDir.up-playerDir.down>-10
        && !myMap.checkAllCollisionsE({myPlayer.xCoord,moveUp,myPlayer.getHitbox().width,myPlayer.getHitbox().height}))
        {
            return movePlayer("up",x,y);
        }
        myPlayer.xCoord=moveRight;
        myPlayer.xVelocity=0;
    }
    if(!strcmp(c,"down"))
    {
        if((objDir.left-playerDir.right>=-1 || playerDir.left-objDir.right>=-1))
        {
            if(objDir.left-playerDir.right>=-1)
                myPlayer.xCoord=moveLeft;
            else
                myPlayer.xCoord=moveRight;
        }
        else
        {
            myPlayer.yCoord=moveDown;
            myPlayer.yVelocity=0;
        }
    }
}


#endif // OBJECTS
