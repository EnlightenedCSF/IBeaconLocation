//
//  AlgorithmSphereIntersection.h
//  IBeaconLocation
//
//  Created by Alexander Olferuk on 25/01/16.
//  Copyright © 2016 Alexander Olferuk. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "BeaconLocation.h"

@interface AlgorithmSphereIntersection : NSObject <TrilaterationAlgorithm>

-(CGPoint)locationWithBeacons:(NSArray<Beacon *> *)beacons;

@end
