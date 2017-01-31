/*
 * DotScene.h
 *
 *  Created on: Apr 1, 2015
 *      Author: cpresser
 */

#ifndef DOTSCENE_H_
#define DOTSCENE_H_


#include <Ogre.h>
#include <vector>
#include "tinyxml.h"
using namespace std;
using namespace Ogre;

class nodeProperty
{
public:
   String nodeName;
   String propertyNm;
   String valueName;

   nodeProperty(String node,String propertyName,String value)
   {
      nodeName = node;
      propertyNm = propertyName;
      valueName = value;
   }
};

Light* LoadLight( TiXmlElement *XMLLight, SceneManager *mSceneMgr )
{
   TiXmlElement *XMLDiffuse, *XMLSpecular, *XMLAttentuation, *XMLPosition;

   // Create a light (point | directional | spot | radPoint)
   Light* l = mSceneMgr->createLight( XMLLight->Attribute("name") );
   if( !XMLLight->Attribute("type") || String(XMLLight->Attribute("type")) == "point" )
      l->setType( Light::LT_POINT );
   else if( String(XMLLight->Attribute("type")) == "directional")
      l->setType( Light::LT_DIRECTIONAL );
   else if( String(XMLLight->Attribute("type")) == "spot")
      l->setType( Light::LT_SPOTLIGHT );
   else if( String(XMLLight->Attribute("type")) == "radPoint")
      l->setType( Light::LT_POINT );

   XMLDiffuse = XMLLight->FirstChildElement("colourDiffuse");
   if( XMLDiffuse ){
      ColourValue Diffuse;
      Diffuse.r = Ogre::StringConverter::parseReal( XMLDiffuse->Attribute("r") );
      Diffuse.g = Ogre::StringConverter::parseReal( XMLDiffuse->Attribute("g") );
      Diffuse.b = Ogre::StringConverter::parseReal( XMLDiffuse->Attribute("b") );
      Diffuse.a = 1;
      l->setDiffuseColour(Diffuse);
   }
   XMLSpecular = XMLLight->FirstChildElement("colourSpecular");
   if( XMLSpecular ){
      ColourValue Specular;
      Specular.r = Ogre::StringConverter::parseReal( XMLSpecular->Attribute("r") );
      Specular.g = Ogre::StringConverter::parseReal( XMLSpecular->Attribute("g") );
      Specular.b = Ogre::StringConverter::parseReal( XMLSpecular->Attribute("b") );
      Specular.a = 1;
      l->setSpecularColour(Specular);
   }

   XMLAttentuation = XMLLight->FirstChildElement("lightAttenuation");
   if( XMLAttentuation )
   {
      //get defaults incase not all values specified
      Real range, constant, linear, quadratic;
      range = l->getAttenuationRange();
      constant = l->getAttenuationConstant();
      linear = l->getAttenuationLinear();
      quadratic = l->getAttenuationQuadric();

      if( XMLAttentuation->Attribute("range") )
         range = StringConverter::parseReal( XMLAttentuation->Attribute("range") );
      if( XMLAttentuation->Attribute("constant") )
         constant = StringConverter::parseReal( XMLAttentuation->Attribute("constant") );
      if( XMLAttentuation->Attribute("linear") )
         linear = StringConverter::parseReal( XMLAttentuation->Attribute("linear") );
      if( XMLAttentuation->Attribute("quadratic") )
         quadratic = StringConverter::parseReal( XMLAttentuation->Attribute("quadratic") );

      l->setAttenuation( range, constant, linear, quadratic );
   }

   XMLPosition = XMLLight->FirstChildElement("position");
   if( XMLPosition ) {
      Vector3 p = Vector3(0,0,0);
      if( XMLPosition->Attribute("x") )
         p.x = StringConverter::parseReal( XMLPosition->Attribute("x") );
      if( XMLPosition->Attribute("y") )
         p.y = StringConverter::parseReal( XMLPosition->Attribute("y") );
      if( XMLPosition->Attribute("z") )
         p.z = StringConverter::parseReal( XMLPosition->Attribute("z") );

      l->setPosition( p );
   }

   //castShadows      (true | false) "true"
   l->setCastShadows( true );
   if( XMLLight->Attribute("castShadows") )
      if( String(XMLLight->Attribute("castShadows")) == "false" )
         l->setCastShadows( false );

   //visible         (true | false) "true"
   l->setVisible( true );
   if( XMLLight->Attribute("visible") )
      if( String(XMLLight->Attribute("visible")) == "false" )
         l->setVisible( false );

   return l;
}

std::vector<nodeProperty> nodeProperties;

void parseDotScene( const String &SceneName, const String& groupName, SceneManager *mSceneMgr )
{
   TiXmlDocument   *XMLDoc;
   TiXmlElement   *XMLRoot, *XMLNodes;

   try
   {
      DataStreamPtr pStream = ResourceGroupManager::getSingleton().
         openResource( SceneName, groupName );

      String data = pStream->getAsString();
      // Open the .scene File
      XMLDoc = new TiXmlDocument();
      XMLDoc->Parse( data.c_str() );
      pStream->close();
      pStream.setNull();

      if( XMLDoc->Error() )
      {
         //We'll just log, and continue on gracefully
         LogManager::getSingleton().logMessage("[dotSceneLoader] The TiXmlDocument reported an error");
         delete XMLDoc;
         return;
      }
   }
   catch(...)
   {
      //We'll just log, and continue on gracefully
      LogManager::getSingleton().logMessage("[dotSceneLoader] Error creating TiXmlDocument");
      delete XMLDoc;
      return;
   }

   // Validate the File
   XMLRoot = XMLDoc->RootElement();
   if( String( XMLRoot->Value()) != "scene"  ) {
      LogManager::getSingleton().logMessage( "[dotSceneLoader]Error: Invalid .scene File. Missing <scene>" );
      delete XMLDoc;
      return;
   }

   XMLNodes = XMLRoot->FirstChildElement( "nodes" );

   // Read in the scene nodes
   if( XMLNodes )
   {
      TiXmlElement *XMLNode, *XMLPosition, *XMLRotation, *XMLScale,  *XMLEntity, *XMLBillboardSet,  *XMLLight, *XMLUserData;

      XMLNode = XMLNodes->FirstChildElement( "node" );

      while( XMLNode )
      {
         // Process the current node
         // Grab the name of the node
         String NodeName = XMLNode->Attribute("name");
         // First create the new scene node
         SceneNode* NewNode = mSceneMgr->getRootSceneNode()->createChildSceneNode( NodeName );
         Vector3 TempVec;
         String TempValue;

         // Now position it...
         XMLPosition = XMLNode->FirstChildElement("position");
         if( XMLPosition ){
            TempValue = XMLPosition->Attribute("x");
            TempVec.x = StringConverter::parseReal(TempValue);
            TempValue = XMLPosition->Attribute("y");
            TempVec.y = StringConverter::parseReal(TempValue);
            TempValue = XMLPosition->Attribute("z");
            TempVec.z = StringConverter::parseReal(TempValue);
            NewNode->setPosition( TempVec );
         }

         // Rotate it...
         XMLRotation = XMLNode->FirstChildElement("rotation");
         if( XMLRotation ){
            Quaternion TempQuat;
            TempValue = XMLRotation->Attribute("qx");
            TempQuat.x = StringConverter::parseReal(TempValue);
            TempValue = XMLRotation->Attribute("qy");
            TempQuat.y = StringConverter::parseReal(TempValue);
            TempValue = XMLRotation->Attribute("qz");
            TempQuat.z = StringConverter::parseReal(TempValue);
            TempValue = XMLRotation->Attribute("qw");
            TempQuat.w = StringConverter::parseReal(TempValue);
            NewNode->setOrientation( TempQuat );
         }

         // Scale it.
         XMLScale = XMLNode->FirstChildElement("scale");
         if( XMLScale ){
            TempValue = XMLScale->Attribute("x");
            TempVec.x = StringConverter::parseReal(TempValue);
            TempValue = XMLScale->Attribute("y");
            TempVec.y = StringConverter::parseReal(TempValue);
            TempValue = XMLScale->Attribute("z");
            TempVec.z = StringConverter::parseReal(TempValue);
            NewNode->setScale( TempVec );
         }

         XMLLight = XMLNode->FirstChildElement( "light" );
         if( XMLLight )
            NewNode->attachObject( LoadLight( XMLLight, mSceneMgr ) );

         // Check for an Entity
         XMLEntity = XMLNode->FirstChildElement("entity");
         if( XMLEntity )
         {
            String EntityName, EntityMeshFilename;
            EntityName = XMLEntity->Attribute( "name" );
            EntityMeshFilename = XMLEntity->Attribute( "meshFile" );

            // Create entity
            Entity* NewEntity = mSceneMgr->createEntity(EntityName, EntityMeshFilename);

            //castShadows      (true | false) "true"
        NewEntity->setCastShadows( true );
        if( XMLEntity->Attribute("castShadows") )
               if( String(XMLEntity->Attribute("castShadows")) == "false" )
                  NewEntity->setCastShadows( false );

            NewNode->attachObject( NewEntity );
         }

         XMLBillboardSet = XMLNode->FirstChildElement( "billboardSet" );
         if( XMLBillboardSet )
         {
            String TempValue;

            BillboardSet* bSet = mSceneMgr->createBillboardSet( NewNode->getName() );

            BillboardType Type;
            TempValue = XMLBillboardSet->Attribute( "type" );
            if( TempValue == "orientedCommon" )
               Type = BBT_ORIENTED_COMMON;
            else if( TempValue == "orientedSelf" )
               Type = BBT_ORIENTED_SELF;
            else Type = BBT_POINT;

            BillboardOrigin Origin;
            TempValue = XMLBillboardSet->Attribute( "type" );
            if( TempValue == "bottom_left" )
               Origin = BBO_BOTTOM_LEFT;
            else if( TempValue == "bottom_center" )
               Origin = BBO_BOTTOM_CENTER;
            else if( TempValue == "bottomRight"  )
               Origin = BBO_BOTTOM_RIGHT;
            else if( TempValue == "left" )
               Origin = BBO_CENTER_LEFT;
            else if( TempValue == "right" )
               Origin = BBO_CENTER_RIGHT;
            else if( TempValue == "topLeft" )
               Origin = BBO_TOP_LEFT;
            else if( TempValue == "topCenter" )
               Origin = BBO_TOP_CENTER;
            else if( TempValue == "topRight" )
               Origin = BBO_TOP_RIGHT;
            else
               Origin = BBO_CENTER;

            bSet->setBillboardType( Type );
            bSet->setBillboardOrigin( Origin );

            TempValue = XMLBillboardSet->Attribute( "name" );
            bSet->setMaterialName( TempValue );

            int width, height;
            width = (int) StringConverter::parseReal( XMLBillboardSet->Attribute( "width" ) );
            height = (int) StringConverter::parseReal( XMLBillboardSet->Attribute( "height" ) );
            bSet->setDefaultDimensions( width, height );
            bSet->setVisible( true );
            NewNode->attachObject( bSet );

            TiXmlElement *XMLBillboard;

            XMLBillboard = XMLBillboardSet->FirstChildElement( "billboard" );

            while( XMLBillboard )
            {
               Billboard *b;
               // TempValue;
               TempVec = Vector3( 0, 0, 0 );
               ColourValue TempColour(1,1,1,1);

               XMLPosition = XMLBillboard->FirstChildElement( "position" );
               if( XMLPosition ){
                  TempValue = XMLPosition->Attribute("x");
                  TempVec.x = StringConverter::parseReal(TempValue);
                  TempValue = XMLPosition->Attribute("y");
                  TempVec.y = StringConverter::parseReal(TempValue);
                  TempValue = XMLPosition->Attribute("z");
                  TempVec.z = StringConverter::parseReal(TempValue);
               }

               TiXmlElement* XMLColour = XMLBillboard->FirstChildElement( "colourDiffuse" );
               if( XMLColour ){
                  TempValue = XMLColour->Attribute("r");
                  TempColour.r = StringConverter::parseReal(TempValue);
                  TempValue = XMLColour->Attribute("g");
                  TempColour.g = StringConverter::parseReal(TempValue);
                  TempValue = XMLColour->Attribute("b");
                  TempColour.b = StringConverter::parseReal(TempValue);
               }

               b = bSet->createBillboard( TempVec, TempColour);

               XMLBillboard = XMLBillboard->NextSiblingElement( "billboard" );
            }
         }

         XMLUserData = XMLNode->FirstChildElement( "userData" );
         if ( XMLUserData )
         {
            TiXmlElement *XMLProperty;
            XMLProperty = XMLUserData->FirstChildElement("property");
            while ( XMLProperty )
            {
               String first = NewNode->getName();
               String second = XMLProperty->Attribute("name");
               String third = XMLProperty->Attribute("data");
               nodeProperty newProp(first,second,third);
               nodeProperties.push_back(newProp);
               XMLProperty = XMLProperty->NextSiblingElement("property");
            }
         }

         // Move to the next node
         XMLNode = XMLNode->NextSiblingElement( "node" );
      }
   }

   // Close the XML File
   delete XMLDoc;
}

String getProperty(String ndNm, String prop)
{
   bool could = false;
   for ( unsigned int i = 0 ; i < nodeProperties.size(); i++ )
   {
      if ( nodeProperties[i].nodeName == ndNm && nodeProperties[i].propertyNm == prop )
      {
         return nodeProperties[i].valueName;
         could = true;
      }
   }
   if (could == false)
      return " ";
}

#endif /* DOTSCENE_H_ */
