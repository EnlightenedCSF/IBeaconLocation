# IBeaconLocation

This is a project containing both library files and an example to demostrate its usage.

## Table of contents

* [Library overview](#library-overview)
  * [BeaconLocation](#beaconlocation)
  * [Floor](#floor)
  * [Beacon](#beacon)
  * [Processor](#processor)
  * [Attractor and Spots](#attractor-and-spots)
  * [Algorithms](#algorithms)
* [Error handling](#error-handling)
* [Example](#example)


## Library overview

Library contains following classes:

```
  / .
    |  BeaconLocation
    |  Floor
    |  Beacon
    |  Utils
    |  Processor
    |  Attractor
    |  Spot
    / Algorithms
      / Wrappers
        |  AlgorithmPowerCenter
        |  AlgorithmEPTA
        |  AlgorithmSphereIntersection
      / Implementations
        |  Epta
        |  PowerCenter
        |  SphereIntersection
```

### BeaconLocation

`BeaconLocation` is an entry point for the library and you should start working with the initialization of this class:

```objc
- (instancetype)initWithUUID:(NSUUID *)uuid identifier:(NSString *)identifier;
- (instancetype)initWithUUIDString:(NSString *)uuidString identifier:(NSString *)identifier;

@property (nonatomic, strong) Floor *floor;
@property (nonatomic, strong) Processor *processor;
```

After the initialization, the library creates necessary CoreLocation files (`CLBeaconRegion`, `CLLocationManager`) and handles `CLLocationManager`'s delegate. Moreover, it creates `Floor` object, which stores beacon identifiers and coordinates; and creates `Processor` object, which takes responsibility for the calculation of user position with beacons stored in `Floor` and their accuracies. 

`BeaconLocation` has a delegate of type `BeaconLocationDelegate`, which should contain the following selector:

```objc
@protocol BeaconLocationDelegate <NSObject>
- (void)onUpdateUserPosition:(CGPoint)position;
@end
```

### Floor

`Floor` class contains following methods for adding, getting and removing beacons:

```objc
- (void)addBeaconWithMajor:(NSUInteger)major minor:(NSUInteger)minor xCoord:(double)x yCoord:(double)y;
- (void)addBeaconWithMajor:(NSUInteger)major minor:(NSUInteger)minor xCoord:(double)x yCoord:(double)y zCoord:(double)z;

- (void)addBeaconWithMajor:(NSUInteger)major minor:(NSUInteger)minor inPosition:(CGPoint)point;

- (BOOL)hasBeaconWithMajor:(NSUInteger)major minor:(NSUInteger)minor;
- (Beacon *)beaconWithMajor:(NSUInteger)major minor:(NSUInteger)minor;

- (void)removeAllBeacons;
- (void)removeBeaconWithMajor:(NSUInteger)major minor:(NSUInteger)minor;
```

We assume that you provide coordinates in meters, as the CoreLocation framework itself uses default RSSI and measured RSSI to calculate `accuracy` in meters.

### Beacon

`Beacon` is a storage for beacon's location and its identifiers. 

```objc
@property (nonatomic, assign) NSUInteger major;
@property (nonatomic, assign) NSUInteger minor;

@property (nonatomic, assign) double x;
@property (nonatomic, assign) double y;
@property (nonatomic, assign) double z;

@property (nonatomic, assign) double accuracy;

- (instancetype)initWithMajor:(NSUInteger)major minor:(NSUInteger)minor x:(double)x y:(double)y z:(double)z;
- (void)updateAccuracy:(double)accuracy;
```

You don't need to update accuracy yourself, the library takes this responsibility.

### Processor

`Processor` is the core of the library. By default, it subscribes to `-locationManager:didRangeBeacons:inRegion` event, and calcutates user position with every call.

As declared in `BeaconLocation.h`, the framework offers you 3 different algorithms to choose from, and you can also specify your own method with `customAlgorithm` property stored in `Processor`. 

Each algorithm or even the combination of algorithms can be chosen to achieve the best results. Consider the following interface:

```objc
- (void)setAlgorithm:(AlgorithmType)algorithmType;
- (void)setAlgorithmsAndTrusts:(NSDictionary<NSNumber *, NSNumber *> *)algorithms;
- (void)setAlgorithms:(NSArray<NSNumber *> *)algorithmTypes;
```

The **first** method enables only one algorithm type: `AlgorithmTypeEPTA`, `AlgorithmTypePowerCenter`, `AlgorithmTypeSphereIntersection`, and `AlgorithmTypeCustom` (which are declared in `AlgorithmType` enum in `BeaconLocation.h`). 
> **Pay attention**: you must provide a `customAlgorithm` instance _before_ setting custom algorithm usage with any of the `-setAlgorithm:...` methods!

The **second** one is more tricky. `NSDictionary` that takes the method as the input argument should contain pair with `AlgorithmType` (boxed in a `NSNumber *` object) and the value called _trust_ (also a `NSNumber *`). _Trusts_ assigned to different algorithms show the importance of the result calculated with each of the enabled algorithm.

Let us say we have algorithm (1) and (2). The first one calculates user position to be `(1;1)`, while the second one suggests `(5;5)`. Then if _trusts_ are equal, the result will be averaged, and we will have `(3;3)`. If the second algorithm is twice as important as the first, we will have `0.66 * (5;5) + 0.33 * (1;1) == (3.66; 3.66)`.

Now back to usage. You may choose any positive values you want, as the library performs normalization by 1. Consider the following example:

```objc
self.lib = [[BeaconLocation alloc] initWith...];
// power center type is 3 times more important:
[self.lib.processor setAlgorithmsAndTrusts:@{ @(AlgorithmTypeEPTA): @(1),       // will be 0.25
                                              @(AlgorithmTypePowerCenter): @(3) // will be 0.75
                                              }];
```

> **Note**: if all trusts were assigned to zero, it will be treated as even values.

The **third** one allows you to pick several methods at once; all of them will be used with equal trusts.

### Attractor and Spots

To make your application more location-responsive, the library contains `Spot` and `Attractor` classes.
The `Spot` is an object that represents a 'hot spot', an exactly place where you expect users to be more often than in others. It can be museum exhibits, stocks in markets, gates in airports, and so on.
Managing a `Spot` is pretty straightforward. You can create it as anonymous or give the spot a name.

```objc
@property (nonatomic, strong) NSString *name;

@property (nonatomic, assign) CGFloat x;
@property (nonatomic, assign) CGFloat y;

- (instancetype)initWithX:(CGFloat)x y:(CGFloat)y;
- (instancetype)initWithPoint:(CGPoint)point;
- (instancetype)initWithX:(CGFloat)x y:(CGFloat)y name:(NSString *)name;
- (instancetype)initWithPoint:(CGPoint)point name:(NSString *)name;
```

You can work with spots defined in your app through the `Attractor` interface given below:

```objc
@property (strong, nonatomic) NSMutableArray<Spot *> *spots;

- (void)addSpot:(Spot *)spot;
- (void)removeAllSpots;
- (void)removeLastSpot;
- (void)removeSpotNamed:(NSString *)name;
- (void)removeAllSpotsNamed:(NSString *)name;

//--------------------------------------------------

@property (nonatomic, assign) CGFloat attractionPower;  // 0.1 by default
@property (nonatomic, assign) CGFloat deadZone;         // in meters; 0.2 by default

- (CGPoint)updateUserPosition:(CGPoint)userPosition;
```

Next, after user's position is calculated by `Processor`, it can be attracted to the nearest spot with some power defined with `attractionPower` property. After the attraction is done, it is supposed that user will be provided with the location-based notification sooner, which make the application more responsive and user friendly.

The `deadZone` property is the minimum distance between the user and the spot. The attraction power will not affect location of user if he is already stands close enough to the `Spot` object.  

### Algorithms

The `Algorithm/Implementations` group contains different implementations of the trilateration algorithms written in pure C.

1. **EPTA** (Enhanced Positioning Trilateration Algorithm).
   Implementation of the algorithm based on the research of Peter Brida and Juraj Machaj "A Novel Enhanced Positioning Trilateration Algorithm Implemented for Medical Implant In-Body Localization" which can be found [on this web-page](http://www.hindawi.com/journals/ijap/2013/819695/).

2. **Power Center**.
   Implementation of the algorithm based on the research of V. Pierlot, M. Urbin-Choffray, and M. Van Droogenbroeck "A new three object triangulation algorithm based on the power center of three circles", which can be found [here](http://www.telecom.ulg.ac.be/publi/publications/pierlot/Pierlot2011ANewThreeObject/index.html).

3. **Sphere Intersection**.
   The algorithm is based on the Wikipedia article devoted to [Trilateration](https://en.wikipedia.org/wiki/Trilateration) problem. 

The `Algorithm/Wrappers` group contains Objective-C wrappers of C classes listed above.

## Error handling

To make development easier, the framework uses `NSAssert(...)` macros to validate input arguments.

## Example

So, let us write a simple example demonstrating the possible usage of the framework:

```objc
#import <BeaconLocation/BeaconLocation.h>
@import CoreGraphics;

// ...

@property (nonatomic, strong) BeaconLocation *library;

// ...

@interface MyClass: NSObject <BeaconLocationDelegate>

// ...

- (void)init {
  self = [super init];
  if (self) {
    //init
    _library = [[BeaconLocation alloc] initWithUUIDString:@"12345678-1234-0000-4321-876543210000" 
                                               identifier:@"My region"];
    
    //beacons
    [_library.floor addBeaconWithMajor:0 minor:0 inPosition:CGPointMake(1, 2)];
    [_library.floor addBeaconWithMajor:0 minor:1 inPosition:CGPointMake(2, 3)];
    [_library.floor addBeaconWithMajor:0 minor:2 inPosition:CGPointMake(1, 3)];
    
    //method
    [_library.processor setAlgorithm:AlgorithmTypeSphereIntersection];
    
    //delegate
    _library.delegate = self;
  }
  return self;
}

- (void)onUpdateUserPostion:(CGPoint)position {
  // do fancy stuff
}
```

So that's it.
