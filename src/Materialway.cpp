/**
 * @file Materialway.cpp
 *
 * @date 31/mar/2014
 * @author Stefano Martina
 */

#include "Materialway.h"
#include "DetectorModule.h"
#include "Tracker.h"
#include "Visitable.h"
#include "InactiveSurfaces.h"
#include "InactiveTube.h"
#include "InactiveRing.h"
#include "InactiveElement.h"
#include "MatCalc.h"
#include "MaterialObject.h"
#include "ConversionStation.h"
#include "Barrel.h"
#include "Endcap.h"
#include "Disk.h"
#include "Layer.h"

#include <ctime>


namespace material {

  Materialway::RodSectionsStation::RodSectionsStation() {}

  Materialway::RodSectionsStation::~RodSectionsStation() {}

  void Materialway::RodSectionsStation::addSection(Materialway::Section* section) {
    sections_.push_back(section);
  }

  void Materialway::RodSectionsStation::setStation(Materialway::Section* station) {
    station_ = station;
  }

  std::vector<Materialway::Section*>& Materialway::RodSectionsStation::getSections() {
    return sections_;
  }

  Materialway::Section* Materialway::RodSectionsStation::getStation() {
    return station_;
  }


  //=====================================================================================================================
  //START Materialway::Boundary
  Materialway::Boundary::Boundary(const Visitable* containedElement, int minZ, int minR, int maxZ, int maxR) :
      containedElement_(containedElement),
      outgoingSection_(nullptr),
      minZ_(minZ),
      maxZ_(maxZ),
      minR_(minR),
      maxR_(maxR) {}

  Materialway::Boundary::Boundary() :
    Boundary(nullptr, 0, 0, 0, 0){}
  Materialway::Boundary::~Boundary() {}


  int Materialway::Boundary::isHit(int z, int r, Direction aDirection) const {
    if (aDirection==HORIZONTAL) {
      if ((minR_<r)&&(maxR_>r)) {
        if (minZ_>z) return minZ_;
        else if (maxZ_>z) return -1;
      }
    } else {
      if ((minZ_<z)&&(maxZ_>z)) {
        if (minR_>r) return minR_;
        else if (maxR_>r) return -1;
      }
    }
    return 0;
  }

  void Materialway::Boundary::minZ(int minZ) {
    minZ_ = minZ;
  }
  void Materialway::Boundary::minR(int minR) {
    minR_ = minR;
  }
  void Materialway::Boundary::maxZ(int maxZ) {
    maxZ_ = maxZ;
  }
  void Materialway::Boundary::maxR(int maxR) {
    maxR_ = maxR;
  }
  void Materialway::Boundary::outgoingSection(Section* outgoingSection) {
    outgoingSection_ = outgoingSection;
  }
  int Materialway::Boundary::minZ() const {
    return minZ_;
  }
  int Materialway::Boundary::minR() const {
    return minR_;
  }
  int Materialway::Boundary::maxZ() const {
    return maxZ_;
  }
  int Materialway::Boundary::maxR() const {
    return maxR_;
  }
  Materialway::Section* Materialway::Boundary::outgoingSection() {
    return outgoingSection_;
  }
  //END Materialway::Boundary
  //=====================================================================================================================
  //START Materialway::Train
  Materialway::Train::Train() :
    destination(nullptr) {}
  Materialway::Train::~Train() {}

  void Materialway::Train::relaseMaterial(Section* section) const {
    for(const Wagon& wagon : wagons) {
      if (section->inactiveElement() != nullptr) {
        std::string tmp = wagon.material+"_HV lineeee";
        section->inactiveElement()->addLocalMass(wagon.material, tmp, wagon.droppingGramsMeter * undiscretize(section->lenght()) / 1000.);   //TODO: control if unit is mm
      }
    }
  }

  void Materialway::Train::addWagon(std::string massName, double massQuantity, UnitType massUnit) {
    switch (massUnit) {
    case GRAMS_METERS:
      Wagon newWagon(massName, massQuantity); //TODO:control if base unit is cm or mm
      wagons.push_back(newWagon);
    }
  }
  //END Materialway::Train
  //=====================================================================================================================
  //START Materialway::Section
  Materialway::Section::Section(int minZ, int minR, int maxZ, int maxR, Direction bearing, Section* nextSection, bool debug) :
    minZ_(minZ),
    minR_(minR),
    maxZ_(maxZ),
    maxR_(maxR),
    bearing_(bearing),
    nextSection_(nextSection),
    inactiveElement_(nullptr),
    debug_(debug),
    materialObject_ (MaterialObject::SERVICE) {}

  Materialway::Section::Section(int minZ, int minR, int maxZ, int maxR, Direction bearing, Section* nextSection) :
    Section(minZ, minR, maxZ, maxR, bearing, nextSection, false) {}

  Materialway::Section::Section(int minZ, int minR, int maxZ, int maxR, Direction bearing) :
    Section(minZ, minR, maxZ, maxR, bearing, nullptr) {}
  Materialway::Section::~Section() {}

  int Materialway::Section::isHit(int z, int r, int end, Direction aDirection) const {
    if (aDirection==HORIZONTAL) {
      if ((minR() - sectionWidth - safetySpace < r)&&(maxR() + sectionWidth + safetySpace > r)) {
        if (minZ_>z){
          if (minZ() <= end + safetySpace) {
            return minZ();
          }
        } else if (maxZ()>z) {
          return -1;
        }
      }
    } else {
      if ((minZ() - sectionWidth - safetySpace < z)&&(maxZ() + sectionWidth + safetySpace > z)) {
        if (minR()>r) {
          if (minR() <= end + safetySpace) {
            return minR();
          }
        } else if (maxR()>r) {
          return -1;
        }
      }
    }
    return 0;
  }

  void Materialway::Section::minZ(int minZ){
    minZ_ = minZ;
  }
  void Materialway::Section::minR(int minR){
    minR_ = minR;
  }
  void Materialway::Section::maxZ(int maxZ){
    maxZ_ = maxZ;
  }
  void Materialway::Section::maxR(int maxR){
    maxR_ = maxR;
  }
  void Materialway::Section::bearing(Direction bearing) {
    bearing_ = bearing;
  }
  void Materialway::Section::nextSection(Section* nextSection) {
    nextSection_ = nextSection;
  }

  int Materialway::Section::minZ() const {
    return minZ_;
  }
  int Materialway::Section::minR() const {
    return minR_;
  }
  int Materialway::Section::maxZ() const {
    return maxZ_;
  }
  int Materialway::Section::maxR() const {
    return maxR_;
  }
  int Materialway::Section::lenght() const {
    if (bearing() == HORIZONTAL) {
      return maxZ() - minZ();
    } else {
      return maxR() - minR();
    }
  }
  Materialway::Direction Materialway::Section::bearing() const {
    return bearing_;
  }
  Materialway::Section* Materialway::Section::nextSection() const {
    return nextSection_;
  }
  bool Materialway::Section::hasNextSection() const {
    return (nextSection_ == nullptr)? false : true;
  }

  MaterialObject& Materialway::Section::materialObject() {
    return materialObject_;
  }

  void Materialway::Section::inactiveElement(InactiveElement* inactiveElement) {
    inactiveElement_ = inactiveElement;
  }
  InactiveElement* Materialway::Section::inactiveElement() const {
    return inactiveElement_;
  }
  void Materialway::Section::route(const Train& train) {
    train.relaseMaterial(this);
    if(hasNextSection()) {
      nextSection()->route(train);
    }
  }
  void Materialway::Section::getServicesAndPass(const MaterialObject& source) {
    source.copyServicesTo(materialObject());
    if(hasNextSection()) {
      nextSection()->getServicesAndPass(source);
    }
  }
  //END Materialway::Section
  //=====================================================================================================================
  //START Materialway::Station

  Materialway::Station::Station(int minZ, int minR, int maxZ, int maxR, Direction bearing, ConversionStation& conversionStation, Section* nextSection) :
      Section(minZ, minR, maxZ, maxR, bearing, nextSection),
      conversionStation_ (conversionStation) {}

  Materialway::Station::Station(int minZ, int minR, int maxZ, int maxR, Direction bearing, ConversionStation& conversionStation) :
      Station(minZ, minR, maxZ, maxR, bearing, conversionStation, nullptr) {}
  Materialway::Station::~Station() {}

  void Materialway::Station::route(const Train& train) {
    Section::route(train);
    //inactiveElement()->addLocalMass("Steel", 10000.0); //TODO:cancel
  }

  void Materialway::Station::getServicesAndPass(const MaterialObject& source) {
    return; //PROVA!
    source.copyServicesTo(conversionStation_);
    //don't pass
  }

  ConversionStation& Materialway::Station::conversionStation() {
    return conversionStation_;
  }


  //END Materialway::Station
  //=====================================================================================================================
  //START Materialway::OuterUsher
  Materialway::OuterUsher::OuterUsher(SectionVector& sectionsList, BoundariesSet& boundariesList) :
    sectionsList_(sectionsList),
    boundariesList_(boundariesList) {}
  Materialway::OuterUsher::~OuterUsher() {}

  void Materialway::OuterUsher::go(Boundary* boundary, const Tracker& tracker, Direction direction) {
    int startZ, startR, collision, border;
    bool foundBoundaryCollision, noSectionCollision;
    Section* lastSection = nullptr;
    Section* firstSection = nullptr;

    startZ = boundary->maxZ();
    startR = boundary->maxR();

    bool going=true;
    while (going) {
      foundBoundaryCollision = findBoundaryCollision(collision, border, startZ, startR, tracker, direction);
      noSectionCollision = buildSectionPair(firstSection, lastSection, startZ, startR, collision, border, direction);

      going = foundBoundaryCollision && noSectionCollision;
    }

    boundary->outgoingSection(firstSection);
  }

  /**
   * Look for the nearest collision in every defined boundary, starting from the (startZ, startR) point.
   * If no collision is found return the external perimeter coordinates
   * @param collision is a reference for the return value of the coordinate of the collision point (Z if horizontal, rho if vertical)
   * @param border is a reference for the return value of the coordinate of the border of boundary (rho if horizontal, Z if vertical)
   * @param startZ is the Z coordinate of the starting point
   * @param startR is the rho coordinate of the starting point
   * @param direction
   * @return true if a collision is found, false otherwise
   */
  bool Materialway::OuterUsher::findBoundaryCollision(int& collision, int& border, int startZ, int startR, const Tracker& tracker, Direction direction) {
    int hitCoord;
    int globalMaxZ = discretize(tracker.maxZ()) + globalMaxZPadding;
    int globalMaxR = discretize(tracker.maxR()) + globalMaxRPadding;
    std::map<int, BoundariesSet::const_iterator> hitBoundariesCoords;
    bool foundCollision = false;

    //test all the boundaries for an hit (keep it on a map ordered for key = collision coords, so the first is the nearest collision)
    //for (Boundary& currBoundary : boundariesList_) {
    for(BoundariesSet::const_iterator it = boundariesList_.cbegin(); it != boundariesList_.cend(); ++it) {
      hitCoord = (*it)->isHit(startZ, startR, direction);
      if (hitCoord>0) {
        hitBoundariesCoords[hitCoord] = it;
      }
    }

    if (hitBoundariesCoords.size()) {
      collision = hitBoundariesCoords.begin()->first;
      if(direction == HORIZONTAL) {
        border = (*hitBoundariesCoords.begin()->second)->maxR();
      } else {
        border = (*hitBoundariesCoords.begin()->second)->maxZ();
      }
      foundCollision = true;
    } else {
      if(direction == HORIZONTAL) {
        collision = globalMaxZ;
        border = globalMaxR + safetySpace;
      } else {
        collision = globalMaxR;
        border = globalMaxZ + safetySpace;
      }
    }

    return foundCollision;
  }

  /**
   * Look for the nearest collision in every defined section, starting from the (startZ, startR) point.
   * @param sectionCollision is a reference for the return value of the coordinate of the collision point (Z if horizontal, rho if vertical)
   * and the pointer to the collided section
   * @param startZ is the Z coordinate of the starting point
   * @param startR is the rho coordinate of the starting point
   * @param end is the maximum range of searching for a collision, around (startZ, startR)
   * @param direction
   * @return true if a collision is found, false otherwise
   */
  bool Materialway::OuterUsher::findSectionCollision(std::pair<int,Section*>& sectionCollision, int startZ, int startR, int end, Direction direction) {
    int hitCoord;
    std::map<int, Section*> hitSectionsCoords;

    //test all the sections for an hit (keep it on a map ordered for key = collision coords, so the first is the nearest collision)
    for (Section* currSection : sectionsList_) {
      hitCoord = currSection->isHit(startZ, startR, end, direction);
      if (hitCoord>0) {
        hitSectionsCoords[hitCoord] = currSection;
      }
    }

    if (hitSectionsCoords.size()) {
      sectionCollision = std::make_pair(hitSectionsCoords.begin()->first, hitSectionsCoords.begin()->second);
      return true;
    }
    return false;
  }

  /**
   * Build a section segment from the start point to the end in given direction. Also update the value of the start
   * with the new start point
   * @param firstSection is a pointer for returning the first built section
   * @param lastSection is a pointer to the previous section, is used for updating the nextSection pointer to this
   * @param startZ is a reference to the Z start coordinate
   * @param startR is a reference to the rho start coordinate
   * @param end is a reference to the end coordinate (Z if horizontal, rho if vertical)
   * @param direction is the direction
   * @return true if no section collision found, false otherwise
   */
  bool Materialway::OuterUsher::buildSection(Section*& firstSection, Section*& lastSection, int& startZ, int& startR, int end, Direction direction) {
    int minZ, minR, maxZ, maxR;
    int trueEnd = end;
    int cutCoordinate;
    Section* newSection;
    bool foundSectionCollision = false;
    bool returnValue = true;
    std::pair<int,Section*> sectionCollision;

    //search for collisions
    foundSectionCollision = findSectionCollision(sectionCollision, startZ, startR, end, direction);

    //if section collision found update the end
    if(foundSectionCollision) {
      trueEnd = sectionCollision.first - safetySpace;
      returnValue = false;
    }

    //set coordinates
    if (direction == HORIZONTAL) {
      minZ = startZ;
      minR = startR;
      maxZ = trueEnd;
      maxR = startR + sectionWidth;

      startZ = trueEnd + safetySpace;
    } else {
      minZ = startZ;
      minR = startR;
      maxZ = startZ + sectionWidth;
      maxR = trueEnd;

      startR = trueEnd + safetySpace;
    }

    //build section and update last section's nextSection pointer
    if ((maxZ > minZ) && (maxR > minR)) {
      newSection = new Section(minZ, minR, maxZ, maxR, direction);
      sectionsList_.push_back(newSection);
      updateLastSectionPointer(lastSection, newSection);
      lastSection = newSection;
      //if firstSection is not yet set, set it
      if(firstSection == nullptr) {
        firstSection = newSection;
      }
    }

    //if found a collision and the section is perpendicular split the collided section and update nextSection pointer
    if(foundSectionCollision) {
      if (sectionCollision.second->bearing() != direction) {
        if (direction == HORIZONTAL) {
          cutCoordinate = minR;
        } else {
          cutCoordinate = minZ;
        }
        newSection = splitSection(sectionCollision.second, cutCoordinate, direction);
        sectionsList_.push_back(newSection);
        updateLastSectionPointer(lastSection, newSection);
      } else {
        //set directly the pointer
        updateLastSectionPointer(lastSection, sectionCollision.second);
      }
    }

    return returnValue;
  }

  /**
   * Build a pair of section around a border in the shape of an L, the first section from the start point to the boundary collision,
   * the second section from the collision to the boundary border
   * @param firstSection is a pointer for returning the first built section
   * @param lastSection is a pointer to the previous section, is used for updating the nextSection pointer to this
   * @param startZ is a reference to the Z start coordinate
   * @param startR is a reference to the rho start coordinate
   * @param collision is a coordinate for the first collision point with boundary (Z if horizontal, rho if vertical)
   * @param border is a coordinate for the second point, the border of the boundary (rho if horizontal, Z if vertical)
   * @param direction is the direction
   * @return true if no section collision found, false otherwise
   */
  bool Materialway::OuterUsher::buildSectionPair(Section*& firstSection, Section*& lastSection, int& startZ, int& startR, int collision, int border, Direction direction) {

    bool noCollision = true;

    //build first section and if no collision happened build also the second section
    noCollision = buildSection(firstSection, lastSection, startZ, startR, collision - sectionWidth - safetySpace, direction);
    if (noCollision) {
      noCollision = buildSection(firstSection, lastSection, startZ, startR, border - safetySpace, inverseDirection(direction));
    }

    return noCollision;
  }

  Materialway::Section* Materialway::OuterUsher::splitSection(Section* section, int collision, Direction direction) {
    Section* retValue = nullptr;
    Section* useless = nullptr;
    int secMinZ = section->minZ();
    int secMinR = section->minR();
    int secCollision = collision;

      if (direction == HORIZONTAL) {
        buildSection(useless, retValue, secMinZ, secCollision, section->maxR(), inverseDirection(direction));

        section->maxR(collision - safetySpace);
        updateLastSectionPointer(section, retValue);
      } else {
        buildSection(useless, retValue, secCollision, secMinR, section->maxZ(), inverseDirection(direction));

        section->maxZ(collision - safetySpace);
        updateLastSectionPointer(section, retValue);
      }
      return retValue;
  }

  Materialway::Direction Materialway::OuterUsher::inverseDirection(Direction direction) const {
    return (direction == HORIZONTAL)?VERTICAL:HORIZONTAL;
  }

  void Materialway::OuterUsher::updateLastSectionPointer(Section* lastSection, Section* newSection) {
    if(lastSection != nullptr) {
      lastSection->nextSection(newSection);
    }
  }
  //END Materialway::OuterUsher
  //=====================================================================================================================
  //START Materialway::InnerUsher
  Materialway::InnerUsher::InnerUsher(
      SectionVector& sectionsList,
      StationVector& stationListFirst,
      BarrelBoundaryMap& barrelBoundaryAssociations,
      EndcapBoundaryMap& endcapBoundaryAssociations,
      ModuleSectionMap& moduleSectionAssociations,
      LayerRodSectionsMap& layerRodSections,
      DiskRodSectionsMap& diskRodSections) :
        sectionsList_(sectionsList),
        stationListFirst_(stationListFirst),
        barrelBoundaryAssociations_(barrelBoundaryAssociations),
        endcapBoundaryAssociations_(endcapBoundaryAssociations),
        moduleSectionAssociations_(moduleSectionAssociations),
        layerRodSections_(layerRodSections),
        diskRodSections_(diskRodSections) {}
  Materialway::InnerUsher::~InnerUsher() {}

  void Materialway::InnerUsher::go(Tracker& tracker) {
    //Visitor for the barrel layers
    class MultipleVisitor : public GeometryVisitor {
    public:
      MultipleVisitor(
          SectionVector& sectionsList,
          StationVector& stationListFirst,
          BarrelBoundaryMap& barrelBoundaryAssociations,
          EndcapBoundaryMap& endcapBoundaryAssociations,
          ModuleSectionMap& moduleSectionAssociations,
          LayerRodSectionsMap& layerRodSections,
          DiskRodSectionsMap& diskRodSections) :
        sectionsList_(sectionsList),
        stationListFirst_(stationListFirst),
        barrelBoundaryAssociations_(barrelBoundaryAssociations),
        endcapBoundaryAssociations_(endcapBoundaryAssociations),
        moduleSectionAssociations_(moduleSectionAssociations),
        layerRodSections_(layerRodSections),
        diskRodSections_(diskRodSections),
        startLayer(nullptr),
        //startLayerZMinus(nullptr),
        startBarrel(nullptr),
        startEndcap(nullptr),
        startDisk(nullptr),
        barrelConversionStation_(nullptr),
        currLayer_(nullptr),
        currDisk_(nullptr) {}
        //currEndcapPosition(POSITIVE) {}//, splitCounter(0) {}
      virtual ~MultipleVisitor() {}

      //For the barrels ----------------------------------------------------
      void visit(Barrel& barrel) {
        //build section right to layers
        Boundary* boundary = barrelBoundaryAssociations_[&barrel];

        int minZ = discretize(barrel.maxZ()) + layerSectionRightMargin + safetySpace + layerStationLenght;
        int minR = discretize(barrel.minR());
        int maxZ = minZ + sectionWidth;
        int maxR = boundary->maxR() - safetySpace;

        startBarrel = new Section(minZ, minR, maxZ, maxR, VERTICAL, boundary->outgoingSection()); //TODO:build other segment in other direction?

        barrelConversionStation_ = barrel.conversionStation();
      }

      void visit(Layer& layer) {
        currLayer_ = &layer;
        //split the right section
        Section* section = startBarrel;
        int attachPoint = discretize(layer.maxR()) + layerSectionMargin;        //discretize(layer.minR());

        while(section->maxR() < attachPoint + sectionTolerance) {
          if(!section->hasNextSection()) {
            //TODO: messaggio di errore
            return;
          }
          section = section->nextSection();
        }

        if (section->minR() < attachPoint - sectionTolerance) {
          section = splitSection(section, attachPoint);
        }

        //built two main sections above the layer (one for positive part, one for negative)

        int sectionMinZ = safetySpace;
        int sectionMinR = attachPoint;
        //int sectionMaxZ = discretize(layer.maxZ()) + layerSectionRightMargin;
        int sectionMaxZ = section->minZ() - safetySpace - layerStationLenght;
        int sectionMaxR = sectionMinR + sectionWidth;

        int stationMinZ = sectionMaxZ + safetySpace;
        int stationMinR = sectionMinR -(layerStationWidth/2);
        int stationMaxZ = sectionMaxZ + safetySpace + layerStationLenght;
        int stationMaxR = sectionMinR + (layerStationWidth/2);

        Station* station = new Station(stationMinZ, stationMinR, stationMaxZ, stationMaxR, VERTICAL, *barrelConversionStation_, section); //TODO: check if is ok VERTICAL
        sectionsList_.push_back(station);
        stationListFirst_.push_back(station);
        startLayer = new Section(sectionMinZ, sectionMinR, sectionMaxZ, sectionMaxR, HORIZONTAL, station);
        sectionsList_.push_back(startLayer);
        layerRodSections_[currLayer_].addSection(startLayer);
        layerRodSections_[currLayer_].setStation(station);

        /*
        //sectionMinZ = discretize(layer.sectionMinZ()) - layerSectionRightMargin;
        sectionMinZ = - section->minZ() + safetySpace + layerStationLenght;
        sectionMaxZ = 0 - safetySpace;

        stationMinZ = sectionMinZ - safetySpace - layerStationLenght;
        stationMaxZ = sectionMinZ - safetySpace;

        station = new Station(stationMinZ, stationMinR, stationMaxZ, stationMaxR, VERTICAL); //TODO: check if is ok VERTICAL
        sectionsList_.push_back(station);
        startLayerZMinus = new Section(sectionMinZ, sectionMinR, sectionMaxZ, sectionMaxR, HORIZONTAL, station); //TODO:Togli la sezione inutile (o fai meglio)
        sectionsList_.push_back(startLayerZMinus);
        */

        //TODO:aggiungi riferimento della rod a startZ...
      }

      void visit(BarrelModule& module) {
        Section* section;
        int attachPoint;

        //if the module is in z plus
        if(module.maxZ() > 0) {
          attachPoint = discretize(module.maxZ());
          section = startLayer;
          while (section->maxZ() < attachPoint + sectionTolerance) {
            if(!section->hasNextSection()) {
              //TODO: messaggio di errore
              return;
            }
            section = section->nextSection();
          }
          if (section->minZ() < attachPoint - sectionTolerance) {
            section = splitSection(section, attachPoint);
            layerRodSections_[currLayer_].addSection(section);
          }
          moduleSectionAssociations_[&module] = section;
        }
        /*
        else {
          attachPoint = discretize(module.minZ());
          section = startLayerZMinus;
          while (section->minZ() > attachPoint - sectionTolerance) {
            if(!section->hasNextSection()) {
              //TODO: messaggio di errore
              return;
            }
            section = section->nextSection();
          }
          if (section->maxZ() > attachPoint + sectionTolerance) {
            section = splitSection(section, attachPoint, false);
          }
          moduleSectionAssociations_[&module] = section;
        }
        */
      }

      //For the endcaps ----------------------------------------------------
      void visit(Endcap& endcap) {
        if (endcap.minZ() >= 0) {
          //currEndcapPosition = POSITIVE;

          //build section above the disks
          Boundary* boundary = endcapBoundaryAssociations_[&endcap];
          int minZ = discretize(endcap.minZ());
          int minR = discretize(endcap.maxR())  + diskSectionUpMargin + safetySpace;
          int maxZ = boundary->maxZ() - safetySpace;
          int maxR = minR + sectionWidth;
          startEndcap = new Section(minZ, minR, maxZ, maxR, HORIZONTAL, boundary->outgoingSection()); //TODO:build other segment in other direction?
        }
        //else {
          //currEndcapPosition = NEGATIVE;
        //}
      }

      void visit(Disk& disk) {
        currDisk_ = &disk;
        //if(currEndcapPosition == POSITIVE) {
        if (disk.minZ() >= 0) {
          //split the right section
          Section* section = startEndcap;
          int attachPoint = discretize(disk.maxZ()) + diskSectionMargin;

          while(section->maxZ() < attachPoint + sectionTolerance) {
            if(!section->hasNextSection()) {
              //TODO: messaggio di errore
              return;
            }
            section = section->nextSection();
          }

          if (section->minZ() < attachPoint - sectionTolerance) {
            section = splitSection(section, attachPoint);
          }

          //built two main sections above the layer

          int minZ = attachPoint;
          int minR = discretize(disk.minR());
          int maxZ = minZ + sectionWidth;
          //int maxR = discretize(disk.maxR()) + diskSectionUpMargin;
          int maxR = section->minR() - safetySpace;

          startDisk = new Section(minZ, minR, maxZ, maxR, VERTICAL, section);
          //startDisk = new Section(minZ, minR, maxZ, maxR, VERTICAL);
          sectionsList_.push_back(startDisk);
          diskRodSections_[currDisk_].addSection(startDisk);
          diskRodSections_[currDisk_].setStation(section);

          //TODO:aggiungi riferimento della rod a startZ...
        }
      }

      void visit(EndcapModule& module) {
        //if(currEndcapPosition == POSITIVE) {
        if (module.minZ() >= 0) {
          Section* section;
          int attachPoint;

          attachPoint = discretize(module.maxR());
          section = startDisk;
          while (section->maxR() < attachPoint + sectionTolerance) {
            if(!section->hasNextSection()) {
              //TODO: messaggio di errore
              return;
            }
            section = section->nextSection();
          }
          if (section->minR() < attachPoint - sectionTolerance) {
            section = splitSection(section, attachPoint);
            diskRodSections_[currDisk_].addSection(section);
          }

          moduleSectionAssociations_[&module] = section;
        }
      }

    private:
      enum ZPosition {POSITIVE, NEGATIVE};
      //int splitCounter;
      SectionVector& sectionsList_;
      StationVector& stationListFirst_;
      BarrelBoundaryMap& barrelBoundaryAssociations_;
      EndcapBoundaryMap& endcapBoundaryAssociations_;
      ModuleSectionMap& moduleSectionAssociations_;
      LayerRodSectionsMap& layerRodSections_;  //map each layer with corresponding sections vector and station
      DiskRodSectionsMap& diskRodSections_;
      Section* startLayer;
      //Section* startLayerZMinus;
      Section* startDisk;
      Section* startBarrel;
      Section* startEndcap;
      //ZPosition currEndcapPosition;
      ConversionStation* barrelConversionStation_;
      Layer* currLayer_;
      Disk* currDisk_;


      //Section* findAttachPoint(Section* section, )

      Section* splitSection(Section* section, int collision/*, bool zPlus = true*/, bool debug = false) {
        //std::cout << "SplitSection " << setw(10) << left << ++splitCounter << " ; collision " << setw(10) << left << collision <<" ; section->minZ() " << setw(10) << left << section->minZ() <<" ; section->maxZ() " << setw(10) << left << section->maxZ() <<" ; section->minR() " << setw(10) << left << section->minR() <<" ; section->maxR() " << setw(10) << left << section->maxR() << endl;
        Section* newSection = nullptr;

        if (section->bearing() == HORIZONTAL) {
          //if(zPlus) {
            newSection = new Section(collision, section->minR(), section->maxZ(), section->maxR(), section->bearing(), section->nextSection(), debug);
          //} else {
          //  newSection = new Section(section->minZ(), section->minR(), collision, section->maxR(), section->bearing(), section->nextSection());
          //}
          sectionsList_.push_back(newSection);

          //if(zPlus) {
            section->maxZ(collision - safetySpace);
          //} else {
          //  section->minZ(collision + safetySpace);
          //}
          section->nextSection(newSection);

        } else {
          newSection = new Section(section->minZ(), collision, section->maxZ(), section->maxR(), section->bearing(), section->nextSection(), debug);
          sectionsList_.push_back(newSection);

          section->maxR(collision - safetySpace);
          section->nextSection(newSection);
        }
        return newSection;
      }
    }; //END class MultipleVisitor

    MultipleVisitor visitor (sectionsList_, stationListFirst_, barrelBoundaryAssociations_, endcapBoundaryAssociations_, moduleSectionAssociations_, layerRodSections_, diskRodSections_);
    tracker.accept(visitor);
  }

  //END Materialway::InnerUsher
  //=====================================================================================================================
  //START Materialway

  const double Materialway::gridFactor = 1000.0;                                     /**< the conversion factor for using integers in the algorithm (helps finding collisions),
                                                                              actually transforms millimiters in microns */
  const int Materialway::sectionWidth = discretize(insur::volume_width);     /**< the width of a section */
  const int Materialway::safetySpace = discretize(insur::epsilon);           /**< the safety space between sections */
  //const double Materialway::globalMaxZ_mm = insur::max_length;                     /**< the Z coordinate of the end point of the sections */
  //const double Materialway::globalMaxR_mm = insur::outer_radius;                   /**< the rho coordinate of the end point of the sections */
  //const int Materialway::globalMaxZ = discretize(globalMaxZ_mm);
  //const int Materialway::globalMaxR = discretize(globalMaxR_mm);
  const int Materialway::boundaryPadding = discretize(10.0);             /**< the space between the barrel/endcap and the containing box (for routing services) */
  const int Materialway::boundaryPrincipalPadding = discretize(15.0);       /**< the space between the barrel/endcap and the containing box only right for the barrel, up for endcap */
  const int Materialway::globalMaxZPadding = discretize(100.0);          /**< the space between the tracker and the right limit (for exiting the services) */
  const int Materialway::globalMaxRPadding = discretize(50.0);          /**< the space between the tracker and the upper limit (for exiting the services) */
  const int Materialway::layerSectionMargin = discretize(2.0);          /**< the space between the layer and the service sections over it */
  const int Materialway::diskSectionMargin = discretize(2.0);          /**< the space between the disk and the service sections right of it */
  const int Materialway::layerSectionRightMargin = discretize(5.0);     /**< the space between the end of the layer (on right) and the end of the service sections over it */
  const int Materialway::diskSectionUpMargin = discretize(5.0);     /**< the space between the end of the disk (on top) and the end of the service sections right of it */
  const int Materialway::sectionTolerance = discretize(1.0);       /**< the tolerance for attaching the modules in the layers and disk to the service section next to it */
  const int Materialway::layerStationLenght = discretize(5.0);         /**< the lenght of the converting station on right of the layers */
  const int Materialway::layerStationWidth = discretize(20.0);         /**< the width of the converting station on right of the layers */


  Materialway::Materialway() :
    outerUsher(sectionsList_, boundariesList_),
    innerUsher(sectionsList_, stationListFirst_, barrelBoundaryAssociations_, endcapBoundaryAssociations_, moduleSectionAssociations_, layerRodSections_, diskRodSections_),
    boundariesList_() {}
  Materialway::~Materialway() {}

  int Materialway::discretize(double input) {
    return int(input * gridFactor);
  }
  double Materialway::undiscretize(int input) {
    return double(input / gridFactor);
  }

  bool Materialway::build(Tracker& tracker, InactiveSurfaces& inactiveSurface) {
    /*
    std::cout<<endl<<"tracker: > "<<tracker.maxZ()<<"; v "<<tracker.minR()<<"; ^ "<<tracker.maxR()<<endl;
    std::cout<<"endcap: < "<<tracker.endcaps()[0].minZ()<<"; > "<<tracker.endcaps()[0].maxZ()<<"; v "<<tracker.endcaps()[0].minR()<<"; ^ "<<tracker.endcaps()[0].maxR()<<endl;
    std::cout<<"disk0: < "<<tracker.endcaps()[0].disks()[0].minZ()<<"; > "<<tracker.endcaps()[0].disks()[0].maxZ()<<"; v "<<tracker.endcaps()[0].disks()[0].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[0].maxR()<<endl;
    std::cout<<"disk1: < "<<tracker.endcaps()[0].disks()[1].minZ()<<"; > "<<tracker.endcaps()[0].disks()[1].maxZ()<<"; v "<<tracker.endcaps()[0].disks()[1].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[1].maxR()<<endl;
    std::cout<<"disk2: < "<<tracker.endcaps()[0].disks()[2].minZ()<<"; > "<<tracker.endcaps()[0].disks()[2].maxZ()<<"; v "<<tracker.endcaps()[0].disks()[2].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[2].maxR()<<endl;
    std::cout<<"disk3: < "<<tracker.endcaps()[0].disks()[3].minZ()<<"; > "<<tracker.endcaps()[0].disks()[3].maxZ()<<"; v "<<tracker.endcaps()[0].disks()[3].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[3].maxR()<<endl;
    std::cout<<"disk4: < "<<tracker.endcaps()[0].disks()[4].minZ()<<"; > "<<tracker.endcaps()[0].disks()[4].maxZ()<<"; v "<<tracker.endcaps()[0].disks()[4].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[4].maxR()<<endl;
    std::cout<<"disk5: < "<<tracker.endcaps()[0].disks()[5].minZ()<<"; > "<<tracker.endcaps()[0].disks()[5].maxZ()<<"; v "<<tracker.endcaps()[0].disks()[5].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].maxR()<<endl;
    std::cout<<"disk6: < "<<tracker.endcaps()[0].disks()[6].minZ()<<"; > "<<tracker.endcaps()[0].disks()[6].maxZ()<<"; v "<<tracker.endcaps()[0].disks()[6].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[6].maxR()<<endl;
    std::cout<<"disk7: < "<<tracker.endcaps()[0].disks()[7].minZ()<<"; > "<<tracker.endcaps()[0].disks()[7].maxZ()<<"; v "<<tracker.endcaps()[0].disks()[7].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[7].maxR()<<endl;
    std::cout<<"disk8: < "<<tracker.endcaps()[0].disks()[8].minZ()<<"; > "<<tracker.endcaps()[0].disks()[8].maxZ()<<"; v "<<tracker.endcaps()[0].disks()[8].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[8].maxR()<<endl;
    std::cout<<"disk9: < "<<tracker.endcaps()[0].disks()[9].minZ()<<"; > "<<tracker.endcaps()[0].disks()[9].maxZ()<<"; v "<<tracker.endcaps()[0].disks()[9].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[9].maxR()<<endl;

    std::cout<<"ring0: v "<<tracker.endcaps()[0].disks()[5].rings()[0].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[0].maxR()<<endl;
    std::cout<<"ring1: v "<<tracker.endcaps()[0].disks()[5].rings()[1].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[1].maxR()<<endl;
    std::cout<<"ring2: v "<<tracker.endcaps()[0].disks()[5].rings()[2].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[2].maxR()<<endl;
    std::cout<<"ring3: v "<<tracker.endcaps()[0].disks()[5].rings()[3].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[3].maxR()<<endl;
    std::cout<<"ring4: v "<<tracker.endcaps()[0].disks()[5].rings()[4].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[4].maxR()<<endl;
    std::cout<<"ring5: v "<<tracker.endcaps()[0].disks()[5].rings()[5].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[5].maxR()<<endl;
    std::cout<<"ring6: v "<<tracker.endcaps()[0].disks()[5].rings()[6].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[6].maxR()<<endl;
    std::cout<<"ring7: v "<<tracker.endcaps()[0].disks()[5].rings()[7].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[7].maxR()<<endl;
    std::cout<<"ring8: v "<<tracker.endcaps()[0].disks()[5].rings()[8].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[8].maxR()<<endl;
    std::cout<<"ring9: v "<<tracker.endcaps()[0].disks()[5].rings()[9].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[9].maxR()<<endl;
    std::cout<<"ring10: v "<<tracker.endcaps()[0].disks()[5].rings()[10].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[10].maxR()<<endl;
    std::cout<<"ring11: v "<<tracker.endcaps()[0].disks()[5].rings()[11].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[11].maxR()<<endl;
    std::cout<<"ring12: v "<<tracker.endcaps()[0].disks()[5].rings()[12].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[12].maxR()<<endl;
    std::cout<<"ring13: v "<<tracker.endcaps()[0].disks()[5].rings()[13].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[13].maxR()<<endl;
    std::cout<<"ring14: v "<<tracker.endcaps()[0].disks()[5].rings()[14].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[14].maxR()<<endl;

    std::cout<<"endcapmodule0: < "<<tracker.endcaps()[0].disks()[5].rings()[0].modules()[0].minZ()<<"; > "<<tracker.endcaps()[0].disks()[5].rings()[0].modules()[0].maxZ()<<"; v "<<tracker.endcaps()[0].disks()[5].rings()[0].modules()[0].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[0].modules()[0].maxR()<<endl;
    std::cout<<"endcapmodule1: < "<<tracker.endcaps()[0].disks()[5].rings()[0].modules()[1].minZ()<<"; > "<<tracker.endcaps()[0].disks()[5].rings()[0].modules()[1].maxZ()<<"; v "<<tracker.endcaps()[0].disks()[5].rings()[0].modules()[1].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[0].modules()[1].maxR()<<endl;
    std::cout<<"endcapmodule2: < "<<tracker.endcaps()[0].disks()[5].rings()[0].modules()[2].minZ()<<"; > "<<tracker.endcaps()[0].disks()[5].rings()[0].modules()[2].maxZ()<<"; v "<<tracker.endcaps()[0].disks()[5].rings()[0].modules()[2].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[0].modules()[2].maxR()<<endl;
    std::cout<<"endcapmodule3: < "<<tracker.endcaps()[0].disks()[5].rings()[0].modules()[3].minZ()<<"; > "<<tracker.endcaps()[0].disks()[5].rings()[0].modules()[3].maxZ()<<"; v "<<tracker.endcaps()[0].disks()[5].rings()[0].modules()[3].minR()<<"; ^ "<<tracker.endcaps()[0].disks()[5].rings()[0].modules()[3].maxR()<<endl;

    std::cout<<"===================================================="<<endl;
    std::cout<<"barrel: < "<<tracker.barrels()[0].minZ()<<"; > "<<tracker.barrels()[0].maxZ()<<"; v "<<tracker.barrels()[0].minR()<<"; ^ "<<tracker.barrels()[0].maxR()<<endl;

    std::cout<<"layer0: < "<<tracker.barrels()[0].layers()[0].minZ()<<"; > "<<tracker.barrels()[0].layers()[0].maxZ()<<"; v "<<tracker.barrels()[0].layers()[0].minR()<<"; ^ "<<tracker.barrels()[0].layers()[0].maxR()<<endl;
    std::cout<<"layer1: < "<<tracker.barrels()[0].layers()[1].minZ()<<"; > "<<tracker.barrels()[0].layers()[1].maxZ()<<"; v "<<tracker.barrels()[0].layers()[1].minR()<<"; ^ "<<tracker.barrels()[0].layers()[1].maxR()<<endl;
    std::cout<<"layer2: < "<<tracker.barrels()[0].layers()[2].minZ()<<"; > "<<tracker.barrels()[0].layers()[2].maxZ()<<"; v "<<tracker.barrels()[0].layers()[2].minR()<<"; ^ "<<tracker.barrels()[0].layers()[2].maxR()<<endl;

    std::cout<<"rodpair0: < "<<tracker.barrels()[0].layers()[0].rods()[0].minZ()<<"; > "<<tracker.barrels()[0].layers()[0].rods()[0].maxZ()<<"; v "<<tracker.barrels()[0].layers()[0].rods()[0].minR()<<"; ^ "<<tracker.barrels()[0].layers()[0].rods()[0].maxR()<<endl;
    std::cout<<"rodpair1: < "<<tracker.barrels()[0].layers()[0].rods()[1].minZ()<<"; > "<<tracker.barrels()[0].layers()[0].rods()[1].maxZ()<<"; v "<<tracker.barrels()[0].layers()[0].rods()[1].minR()<<"; ^ "<<tracker.barrels()[0].layers()[0].rods()[1].maxR()<<endl;
    std::cout<<"rodpair2: < "<<tracker.barrels()[0].layers()[0].rods()[2].minZ()<<"; > "<<tracker.barrels()[0].layers()[0].rods()[2].maxZ()<<"; v "<<tracker.barrels()[0].layers()[0].rods()[2].minR()<<"; ^ "<<tracker.barrels()[0].layers()[0].rods()[2].maxR()<<endl;
    std::cout<<"rodpair3: < "<<tracker.barrels()[0].layers()[0].rods()[3].minZ()<<"; > "<<tracker.barrels()[0].layers()[0].rods()[3].maxZ()<<"; v "<<tracker.barrels()[0].layers()[0].rods()[3].minR()<<"; ^ "<<tracker.barrels()[0].layers()[0].rods()[3].maxR()<<endl;

    std::cout<<"barrelModule0: < "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[0].minZ()<<"; > "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[0].maxZ()<<"; v "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[0].minR()<<"; ^ "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[0].maxR()<<endl;
    std::cout<<"barrelModule1: < "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[1].minZ()<<"; > "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[1].maxZ()<<"; v "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[1].minR()<<"; ^ "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[1].maxR()<<endl;
    std::cout<<"barrelModule2: < "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[2].minZ()<<"; > "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[2].maxZ()<<"; v "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[2].minR()<<"; ^ "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[2].maxR()<<endl;
    std::cout<<"barrelModule3: < "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[3].minZ()<<"; > "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[3].maxZ()<<"; v "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[3].minR()<<"; ^ "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[3].maxR()<<endl;
    std::cout<<"barrelModule4: < "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[4].minZ()<<"; > "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[4].maxZ()<<"; v "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[4].minR()<<"; ^ "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[4].maxR()<<endl;
    std::cout<<"barrelModule5: < "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[5].minZ()<<"; > "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[5].maxZ()<<"; v "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[5].minR()<<"; ^ "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[5].maxR()<<endl;
    std::cout<<"barrelModule6: < "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[6].minZ()<<"; > "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[6].maxZ()<<"; v "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[6].minR()<<"; ^ "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[6].maxR()<<endl;
    std::cout<<"barrelModule7: < "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[7].minZ()<<"; > "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[7].maxZ()<<"; v "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[7].minR()<<"; ^ "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[7].maxR()<<endl;
    std::cout<<"barrelModule8: < "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[8].minZ()<<"; > "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[8].maxZ()<<"; v "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[8].minR()<<"; ^ "<<tracker.barrels()[0].layers()[0].rods()[0].modules().first[8].maxR()<<endl;
*/

    bool retValue = false;

    int startTime = time(0);
    if (buildBoundaries(tracker)) {
      std::cout << "TIME " << difftime(time(0), startTime) << " end buildBoundaries" << std::endl;
      buildExternalSections(tracker);
      std::cout << "TIME " << difftime(time(0), startTime) << " end buildExternalSections" << std::endl;
      buildInternalSections(tracker);
      std::cout << "TIME " << difftime(time(0), startTime) << " end buildInternalSections" << std::endl;
    }

    buildInactiveElements();
    std::cout << "TIME " << difftime(time(0), startTime) << " end buildInactiveElements" << std::endl;
    //testTrains();
    //std::cout << "TIME " << difftime(time(0), startTime) << " end testTrains" << std::endl;
    routeServices(tracker);
    std::cout << "TIME " << difftime(time(0), startTime) << " end routeServices" << std::endl;
    //routeModuleServices();
    //std::cout << "TIME " << difftime(time(0), startTime) << " end routeModuleServices" << std::endl;
    //routeRodMaterials();
    //std::cout << "TIME " << difftime(time(0), startTime) << " end routeRodMaterials" << std::endl;
    firstStepConversions();
    std::cout << "TIME " << difftime(time(0), startTime) << " end firstStepConversions" << std::endl;
    createEndCaps(tracker);
    std::cout << "TIME " << difftime(time(0), startTime) << " end createEndCaps" << std::endl;
    populateAllMaterialProperties(tracker);
    std::cout << "TIME " << difftime(time(0), startTime) << " end populateMaterialProperties" << std::endl;
    //calculateMaterialValues(tracker);
    //std::cout << "TIME " << difftime(time(0), startTime) << " end calculateMaterialValues" << std::endl;
    buildInactiveSurface(inactiveSurface);
    std::cout << "TIME " << difftime(time(0), startTime) << " end buildInactiveSurface" << std::endl;
    calculateMaterialValues(inactiveSurface, tracker);
    std::cout << "TIME " << difftime(time(0), startTime) << " end calculateMaterialValues" << std::endl;

    return retValue;
  }

  bool Materialway::buildBoundaries(const Tracker& tracker) {
    bool retValue = false;

    class BarrelVisitor : public ConstGeometryVisitor {
    public:
      BarrelVisitor(BoundariesSet& boundariesList, BarrelBoundaryMap& barrelBoundaryAssociations, EndcapBoundaryMap& endcapBoundaryAssociations) :
        boundariesList_(boundariesList),
        barrelBoundaryAssociations_(barrelBoundaryAssociations),
        endcapBoundaryAssociations_(endcapBoundaryAssociations) {}
      virtual ~BarrelVisitor() {}

      void visit(const Barrel& barrel) {
        int boundMinZ = discretize(barrel.minZ()) - boundaryPadding;
        int boundMinR = discretize(barrel.minR()) - boundaryPadding;
        int boundMaxZ = discretize(barrel.maxZ()) + boundaryPrincipalPadding;
        int boundMaxR = discretize(barrel.maxR()) + boundaryPadding;
        Boundary* newBoundary = new Boundary(&barrel, boundMinZ, boundMinR, boundMaxZ, boundMaxR);

        boundariesList_.insert(newBoundary);
        barrelBoundaryAssociations_.insert(std::make_pair(&barrel, newBoundary));
      }

      void visit(const Endcap& endcap) {
        int boundMinZ = discretize(endcap.minZ()) - boundaryPadding;
        int boundMinR = discretize(endcap.minR()) - boundaryPadding;
        int boundMaxZ = discretize(endcap.maxZ()) + boundaryPadding;
        int boundMaxR = discretize(endcap.maxR()) + boundaryPrincipalPadding;
        Boundary* newBoundary = new Boundary(&endcap, boundMinZ, boundMinR, boundMaxZ, boundMaxR);

        boundariesList_.insert(newBoundary);
        endcapBoundaryAssociations_.insert(std::make_pair(&endcap, newBoundary));
      }

    private:
      BoundariesSet& boundariesList_;
      BarrelBoundaryMap& barrelBoundaryAssociations_;
      EndcapBoundaryMap& endcapBoundaryAssociations_;
    };


    BarrelVisitor visitor (boundariesList_, barrelBoundaryAssociations_, endcapBoundaryAssociations_);
    tracker.accept(visitor);

    if (boundariesList_.size() > 0)
      retValue = true;

    return retValue;
  }

  void Materialway::buildExternalSections(const Tracker& tracker) {
    //for(Boundary& boundary : boundariesList_) {
    int i=0;
    for(BoundariesSet::iterator it = boundariesList_.begin(); it != boundariesList_.end(); ++it) {
      //if(i++==0)
      outerUsher.go(const_cast<Boundary*>(*it), tracker, VERTICAL);
    }
  }

  void Materialway::buildInternalSections(Tracker& tracker) {
    innerUsher.go(tracker);
  }

  void Materialway::buildInactiveElements() {
    double zLength, zOffset, innerRadius, rWidth;

    for(Section* section : sectionsList_) {
    //for(SectionVector::iterator sectionIter = sectionsList_.begin(); sectionIter != sectionsList_.end(); ++sectionIter) {
      if(section->debug_ == false) {
        zLength = undiscretize(section->maxZ() - section->minZ());
        zOffset = undiscretize(section->minZ());
        innerRadius = undiscretize(section->minR());
        rWidth = undiscretize(section->maxR() - section->minR());

        if (section->bearing() == HORIZONTAL) {
          InactiveTube* tube = new InactiveTube;
          tube->setZLength(zLength);
          tube->setZOffset(zOffset);
          tube->setInnerRadius(innerRadius);
          tube->setRWidth(rWidth);
          tube->setFinal(true);
          section->inactiveElement(tube);
          //tube->addLocalMass("Steel", 1000.0*zLength);
          //inactiveSurface.addBarrelServicePart(tube);
        } else {
          InactiveRing* ring = new InactiveRing;
          ring->setZLength(zLength);
          ring->setZOffset(zOffset);
          ring->setInnerRadius(innerRadius);
          ring->setRWidth(rWidth);
          ring->setFinal(true);
          section->inactiveElement(ring);
          //ring->addLocalMass("Steel", 1000.0*rWidth);
          //inactiveSurface.addBarrelServicePart(ring);
        }
      }
    }
  }

  void Materialway::routeServices(const Tracker& tracker) {
    class ServiceVisitor : public ConstGeometryVisitor {
    private:
      ModuleSectionMap& moduleSectionAssociations_;  /**< Map that associate each module with the section that it feeds */
      LayerRodSectionsMap& layerRodSections_;      /**< maps for sections of the rods */
      DiskRodSectionsMap& diskRodSections_;

      const Layer* currLayer_;
      const Disk* currDisk_;

      bool printGuard;
      int printCounter;
    public:
      ServiceVisitor(ModuleSectionMap& moduleSectionAssociations, LayerRodSectionsMap& layerRodSections, DiskRodSectionsMap& diskRodSections) :
        moduleSectionAssociations_(moduleSectionAssociations),
        layerRodSections_(layerRodSections),
        diskRodSections_(diskRodSections), printGuard(true), printCounter(0) {}

      void visit(const Layer& layer) {
        currLayer_ = &layer;
      }

      void visit(const RodPair& rod) {
        for (Section* currSection : layerRodSections_.at(currLayer_).getSections()) {
            rod.materialObject().copyServicesTo(currSection->materialObject());
            rod.materialObject().copyLocalsTo(currSection->materialObject());
        }
        return; //PROVA!
        layerRodSections_.at(currLayer_).getStation()->getServicesAndPass(rod.materialObject());
      }

      void visit(const BarrelModule& module) {
        if(module.maxZ() > 0) {
          moduleSectionAssociations_.at(&module)->getServicesAndPass(module.materialObject());

          return;
          if (printGuard) {
            std::cout << "MODULO:\t\t< "
                      << std::setw(9) << module.minZ() << ";\tv " 
                      << std::setw(9) << module.minR() << ";\t> " 
                      << std::setw(9) << module.maxZ() << ";\t^ " 
                      << std::setw(9) << module.maxR() << std::endl;
            std::cout << "popolo sez mod:\t< " 
                      << std::setw(9) << undiscretize(moduleSectionAssociations_.at(&module)->minZ()) << ";\tv " 
                      << std::setw(9) << undiscretize(moduleSectionAssociations_.at(&module)->minR()) << ";\t> " 
                      << std::setw(9) << undiscretize(moduleSectionAssociations_.at(&module)->maxZ()) << ";\t^ " 
                      << std::setw(9) << undiscretize(moduleSectionAssociations_.at(&module)->maxR()) << std::endl;
          }

          //route layer rod services

          /* NO
          for (
               currSectionIter = layerRodSections_.at(currLayer_).getSections().begin();
               currSectionIter != layerRodSections_.at(currLayer_).getSections().end();
               ++currSectionIter) {
            currLayer_->materialObject().copyServicesTo((*currSectionIter)->materialObject());
            currLayer_->materialObject().copyLocalsTo((*currSectionIter)->materialObject());
          }
          */
         
          /* SI
          RodSectionsStation& rodSectionsStation = layerRodSections_.at(currLayer_);
          std::vector<Section*>& layerSections = rodSectionsStation.getSections();
          for (currSectionIter = layerSections.begin(); currSectionIter != layerSections.end(); ++currSectionIter) {
            Section* currSection = *currSectionIter;
            currLayer_->materialObject().copyServicesTo(currSection->materialObject());
            currLayer_->materialObject().copyLocalsTo(currSection->materialObject());
          }
          */

          //  /* NO
          for (Section* currSection : layerRodSections_.at(currLayer_).getSections()) {
            currLayer_->materialObject().copyServicesTo(currSection->materialObject());
            currLayer_->materialObject().copyLocalsTo(currSection->materialObject());
            if (printGuard) {
              std::cout << "popolo sez rod:\t< " 
                        << std::setw(9) << undiscretize(currSection->minZ()) << ";\tv " 
                        << std::setw(9) << undiscretize(currSection->minR()) << ";\t> " 
                        << std::setw(9) << undiscretize(currSection->maxZ()) << ";\t^ " 
                        << std::setw(9) << undiscretize(currSection->maxR()) << std::endl;
            }
          }
          // */

          /* NO
          RodSectionsStation& rodSectionsStation = layerRodSections_.at(currLayer_);
          std::vector<Section*>& layerSections = rodSectionsStation.getSections();
          for (Section* currSection : layerSections) {
            currLayer_->materialObject().copyServicesTo(currSection->materialObject());
            currLayer_->materialObject().copyLocalsTo(currSection->materialObject());
          }
          */

          //layerRodSections_.at(currLayer_).getStation()->getServicesAndPass(currLayer_->materialObject());
          
          if (printGuard) {
            std::cout << "popolo staz:\t< " 
                      << std::setw(9) << undiscretize(layerRodSections_.at(currLayer_).getStation()->minZ()) << ";\tv " 
                      << std::setw(9) << undiscretize(layerRodSections_.at(currLayer_).getStation()->minR()) << ";\t> " 
                      << std::setw(9) << undiscretize(layerRodSections_.at(currLayer_).getStation()->maxZ()) << ";\t^ " 
                      << std::setw(9) << undiscretize(layerRodSections_.at(currLayer_).getStation()->maxR()) << std::endl;
            
            if (++ printCounter == 2) {
              printGuard = false;
            }
          }
        }
      }

      void visit(const Disk& disk) {
        currDisk_ = &disk;

        /*
        if(currDisk_->minZ() > 0) {
          //iterate for number of radial sectors (module in first ring of disk)
          for (int i = 0; i < currDisk_->rings()[0].modules().size() / 2; ++i) {
            for (Section* currSection : diskRodSections_.at(currDisk_).getSections()) {
              currDisk_->materialObject().copyServicesTo(currSection->materialObject());
              currDisk_->materialObject().copyLocalsTo(currSection->materialObject());
            }
            diskRodSections_.at(currDisk_).getStation()->getServicesAndPass(currDisk_->materialObject());
          }
        }
        */
      }

      void visit(const Ring& ring) {
        //route disk rod services
        if(ring.minZ() > 0) {
          for (Section* currSection : diskRodSections_.at(currDisk_).getSections()) {
            ring.materialObject().copyServicesTo(currSection->materialObject());
            ring.materialObject().copyLocalsTo(currSection->materialObject());
          }
          diskRodSections_.at(currDisk_).getStation()->getServicesAndPass(ring.materialObject());
        }
      }

      void visit(const EndcapModule& module) {
        if (module.minZ() >= 0) {
          //route module services
          moduleSectionAssociations_.at(&module)->getServicesAndPass(module.materialObject());

          /*
          //route disk rod services
          for (Section* currSection : diskRodSections_.at(currDisk_).getSections()) {
            currDisk_->materialObject().copyServicesTo(currSection->materialObject());
            currDisk_->materialObject().copyLocalsTo(currSection->materialObject());
          }
          diskRodSections_.at(currDisk_).getStation()->getServicesAndPass(currDisk_->materialObject());
          */
        }
      }
    };

    ServiceVisitor v(moduleSectionAssociations_, layerRodSections_, diskRodSections_);
    tracker.accept(v);
  }

  /*

  void Materialway::routeModuleServices() {
    for (std::pair<const DetectorModule* const, Section*>& pair : moduleSectionAssociations_) {
      //pair.first->materialObject().routeServicesTo(pair.second->materialObject());
      pair.second->getServicesAndPass(pair.first->materialObject());
    }
  }

  void Materialway::routeRodMaterials() {
    bool first = true;
    for (std::pair<const Layer* const, RodSectionsStation> currAssociation : layerRodSections_) {
      first = true;
      for (Section* currSection : currAssociation.second.getSections()) {
        //currAssociation.first->materialObject().copyServicesTo(currSection->materialObject());
        if (first) {
          currSection->getServicesAndPass(currAssociation.first->materialObject());
          first = false;
        }
        currAssociation.first->materialObject().copyLocalsTo(currSection->materialObject());
      }
      currAssociation.second.getStation()->getServicesAndPass(currAssociation.first->materialObject());
    }

    for (std::pair<const Disk* const, RodSectionsStation> currAssociation : diskRodSections_) {
      for (Section* currSection : currAssociation.second.getSections()) {
        currAssociation.first->materialObject().copyServicesTo(currSection->materialObject());
        currAssociation.first->materialObject().copyLocalsTo(currSection->materialObject());
      }
      currAssociation.second.getStation()->getServicesAndPass(currAssociation.first->materialObject());
    }
  }

  */

  void Materialway::firstStepConversions() {
    for (Station* station : stationListFirst_) {
      if ((station->nextSection() != nullptr) && (station->inactiveElement() != nullptr)) {
        //put local materials in the materialObject of the station
        //put exiting services in the materialObject of the adiacent section of station
        station->conversionStation().routeConvertedElements(station->materialObject(), station->nextSection()->materialObject(), *station->inactiveElement());
        if (station->nextSection()->nextSection() != nullptr) {
          //route from the adiacent section of the adiacent section of the station the materials of the adiacent section of the station
          station->nextSection()->nextSection()->getServicesAndPass(station->nextSection()->materialObject());
        }
      }
      /*
      if (station->materialObject() != nullptr) {
        station->conversionStation().routeConvertedLocalsTo(station->materialObject());
      }

      if (station->nextSection() != nullptr) {
        if (station->nextSection()->materialObject() != nullptr) {
          //put exiting services in the materialObject of the adiacent section of station
          station->conversionStation().routeConvertedServicesTo(station->nextSection()->materialObject());
          if (station->nextSection()->nextSection() != nullptr) {
            //route from the adiacent section of the adiacent section of the station the materials of the adiacent section of the station
            station->nextSection()->nextSection()->getServicesAndPass(station->nextSection()->materialObject());
          }
        }
      }
      */
    }
  }

  void Materialway::createEndCaps(Tracker& tracker) {

    class CapsVisitor : public GeometryVisitor {
    public:
      //std::map<BarrelModule*, int> mappaB;
      //std::map<EndcapModule*, int> mappaE;
      //int totB = 0;
      //int totE = 0;
      CapsVisitor() {}
      void visit(BarrelModule& m) {
        ModuleCap* cap = new ModuleCap(&m);
        cap->setCategory(MaterialProperties::b_mod);
        m.setModuleCap(cap);
        //mappaB[&m] = 0;
        //totB ++;
      }
      void visit(EndcapModule& m) {
        ModuleCap* cap = new ModuleCap(&m);
        cap->setCategory(MaterialProperties::e_mod);
        m.setModuleCap(cap);
        //mappaE[&m] = 0;
        //totE ++;
      }
    };

    CapsVisitor v;
    tracker.accept(v);
    //std::cout << "Barrel " << v.mappaB.size() << " = " << v.totB << "  ----  Endcap " << v.mappaE.size() << " = " << v.totE << std::endl;
  }
 

  void Materialway::populateAllMaterialProperties(Tracker& tracker) {
    //sections
    for(Section* section : sectionsList_) {
      if(section->inactiveElement() != nullptr) {
        section->materialObject().populateMaterialProperties(*section->inactiveElement());
      }
    }

    //modules
    class ModuleVisitor : public GeometryVisitor {
    public:
      ModuleVisitor() {}
      virtual ~ModuleVisitor() {}

      void visit(DetectorModule& module) {
        //ModuleCap* moduleCap = module.getModuleCap();
        //MaterialProperties* materialProperties = ModuleCap;
        //module.materialObject().populateMaterialProperties(*materialProperties);
        module.materialObject().populateMaterialProperties(*module.getModuleCap());
      }
    };

    ModuleVisitor visitor;
    tracker.accept(visitor);
  }

  /*
  void Materialway::calculateMaterialValues(Tracker& tracker) {
    //sections
    for(Section* section : sectionsList_) {
      if(section->inactiveElement() != nullptr) {
        section->inactiveElement()->calculateTotalMass();
        section->inactiveElement()->calculateRadiationLength();
        section->inactiveElement()->calculateInteractionLength();
      }
    }

    //modules
    class ModuleVisitor : public GeometryVisitor {
    public:
      ModuleVisitor() {}
      virtual ~ModuleVisitor() {}

      void visit(DetectorModule& module) {
        ModuleCap* moduleCap = module.getModuleCap();
        moduleCap->calculateTotalMass();
        moduleCap->calculateRadiationLength();
        moduleCap->calculateInteractionLength();
      }
    };

    ModuleVisitor visitor;
    tracker.accept(visitor);
  }
  */

  void Materialway::testTrains() {
    //TODO:erase counter
    std::map<Section*, int> counter;
    for (std::pair<const DetectorModule* const, Section*>& pair : moduleSectionAssociations_) {
      Train train;
      //train.addWagon(Train::GRAMS_METERS, "Cu_HV", 10);
      pair.second->route(train);
      counter[pair.second] ++;
    }

    //for (std::pair<Section* const, int>& pair : counter) {
    //  std::cout<<"Sec (<"<<setw(10)<<left<<pair.first->minZ()<< ")  = "<<pair.second<<endl;
    //}
  }

  void Materialway::buildInactiveSurface(InactiveSurfaces& inactiveSurface) {
    for(Section* section : sectionsList_) {
      if(section->inactiveElement() != nullptr) {
        /*
        if(section->inactiveElement()->getInteractionLength() < 0){
          std::cout<<"ERRORE INT LEN"<<std::endl;
        }
        */
        if((section->inactiveElement()->localMassCount() > 0) && (section->minZ() > 0)) {
          inactiveSurface.addBarrelServicePart(*section->inactiveElement());
          inactiveSurface.addBarrelServicePart(*buildOppositeInactiveElement(section->inactiveElement()));
        }
      }
    }
    /*
    std::vector<InactiveElement>& elements = inactiveSurface.getBarrelServices();
    for (InactiveElement& currElem : elements) {
      if (currElem.getInteractionLength() < 0){
        std::cout<<"ERRORE INT LEN"<<std::endl;
      }
    }
    */
  }

  void Materialway::calculateMaterialValues(InactiveSurfaces& inactiveSurface, Tracker& tracker) {
    //sections
    for (InactiveElement& currElem : inactiveSurface.getBarrelServices()) {
      currElem.calculateTotalMass();
      currElem.calculateRadiationLength();
      currElem.calculateInteractionLength();
    }

    //modules
    class ModuleVisitor : public GeometryVisitor {
    public:
      ModuleVisitor() {}
      virtual ~ModuleVisitor() {}

      void visit(DetectorModule& module) {
        ModuleCap* moduleCap = module.getModuleCap();
        moduleCap->calculateTotalMass();
        moduleCap->calculateRadiationLength();
        moduleCap->calculateInteractionLength();
      }
    };

    ModuleVisitor visitor;
    tracker.accept(visitor);
  }

  InactiveElement* Materialway::buildOppositeInactiveElement(InactiveElement* inactiveElement) {
    InactiveElement* newInactiveElement = new InactiveElement();
    newInactiveElement->setVertical(inactiveElement->isVertical());
    newInactiveElement->setFinal(inactiveElement->isFinal());
    newInactiveElement->setZOffset(-1 * inactiveElement->getZOffset());
    newInactiveElement->setZLength(inactiveElement->getZLength());
    newInactiveElement->setInnerRadius(inactiveElement->getInnerRadius());
    newInactiveElement->setRWidth(inactiveElement->getRWidth());
    newInactiveElement->setTotalMass(inactiveElement->getTotalMass());
    newInactiveElement->setLocalMass(inactiveElement->getLocalMass());

    return newInactiveElement;
  }

  //END Materialway

} /* namespace material */
