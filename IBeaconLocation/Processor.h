//
//  Processor.h
//  IBeaconLocation
//
//  Created by Alexander Olferuk on 23/01/16.
//  Copyright © 2016 Alexander Olferuk. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "BeaconLocation.h"

@import CoreGraphics;

@interface Processor : NSObject

@property (nonatomic, strong) id<TrilaterationAlgorithm> customAlgorithm;
@property (nonatomic, weak) Floor *floor;

- (void)setAlgorithm:(AlgorithmType)algorithmType;
- (void)setAlgorithms:(NSArray<NSNumber *> *)algorithmTypes;
- (void)setAlgorithmsAndTrusts:(NSDictionary<NSNumber *, NSNumber *> *)algorithms;

- (NSMutableDictionary<NSNumber *, NSNumber *> *)algorithmTrusts;
- (NSMutableArray<id<TrilaterationAlgorithm>> *)algorithmImplements;

- (CGPoint)calculateUserPosition;

@end
