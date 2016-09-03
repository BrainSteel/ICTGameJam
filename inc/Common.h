#ifndef COMMON_H
#define COMMON_H

#ifdef DEBUG
#include "stdio.h"
// TODO > Different logging levels? (verbose, toggleable in-game, etc?)
#define gamelog(...) printf(__VA_ARGS__); printf("\n")
#define gamelogerror(...) printf("Error > "); gamelog(__VA_ARGS__)
#define gamelogline gamelog( "Logging line at: %d", __LINE__ )
#else
#define gamelog(...)
#define gamelogerror(...)
#define gamelogline
#endif

#define ROCKET_CONSTANT 10
#define ROCKET_CONSTANT_FACTOR 1
#define PLAYER_BULLET_LIFETIME 100
#define PLAYER_BULLET_SPEED 16
#define PLAYER_BULLET_DAMAGE 3
#define PLAYER_BULLET_RADIUS 5
#define ENEMY_BULLET_LIFETIME 100
#define ENEMY_BULLET_SPEED 13
#define ENEMY_BULLET_DAMAGE 4
#define ENEMY_BULLET_RADIUS 8

/************************************************************************************************
    MANAGED LIST FULL DEFINITIONS AND DECLARATIONS
************************************************************************************************/

#define DeclareManagedList( type )                                                              \
        DeclareManagedListType( type );                                                         \
        DeclareManagedListFunctions( type )

#define DefineManagedList( type )                                                               \
        DefineManagedListType( type );                                                          \
        DefineManagedListFunctions( type )

/************************************************************************************************
    MANAGED LIST TYPE DEFINITION
************************************************************************************************/

#define ManagedListTypeName _ManagedList_
#define ManagedList( type ) ManagedListTypeName##type

#define DeclareManagedListType( type )                                                          \
        typedef struct ManagedListTypeName##type##Struct                                        \
        ManagedListTypeName##type                                                               \

#define DefineManagedListType( type )                                                           \
        struct ManagedListTypeName##type##Struct {                                              \
            type* items;                                                                        \
            unsigned num;                                                                       \
            unsigned firstinactive;                                                             \
            unsigned batchsize;                                                                 \
        }
#undef ManagedListTypeName

/************************************************************************************************
    MANAGED LIST FUNCTION DEFINITIONS
************************************************************************************************/

#define DeclareManagedListFunctions( type )                                                     \
        DeclareManagedListInit( type );                                                         \
        DeclareManagedListFree( type );                                                         \
        DeclareManagedListAlloc( type );                                                        \
        DeclareManagedListUseFirstInactive( type );                                             \
        DeclareManagedListDeactivate( type )

#define DefineManagedListFunctions( type )                                                      \
        DefineManagedListInit( type );                                                          \
        DefineManagedListFree( type );                                                          \
        DefineManagedListAlloc( type );                                                         \
        DefineManagedListUseFirstInactive( type );                                              \
        DefineManagedListDeactivate( type )

/************************************************************************************************
    MANAGED LIST INITIALIZATION
************************************************************************************************/

#define ManagedListInitName _ManagedListInit_
#define ManagedListInit( type, batchsize )                                                      \
        ManagedListInit_##type( batchsize )

#define DeclareManagedListInit( type )                                                          \
        ManagedList( type ) ManagedListInit_##type( int batchsize )                             \

#define DefineManagedListInit( type )                                                           \
        DeclareManagedListInit( type ) {                                                        \
            ManagedList( type ) result;                                                         \
            result.items = NULL;                                                                \
            result.num = 0;                                                                     \
            result.firstinactive = 0;                                                           \
            result.batchsize = batchsize;                                                       \
            return result;                                                                      \
        }
#undef ManagedListInitName

/************************************************************************************************
    MANAGED LIST DESTRUCTION
************************************************************************************************/

#define ManagedListFreeName _ManagedListFree_
#define ManagedListFree( type, ptr ) ManagedListFreeName##type( ptr )

#define DeclareManagedListFree( type )                                                          \
        void ManagedListFreeName##type( ManagedList( type )* ptr )

#define DefineManagedListFree( type )                                                           \
        DeclareManagedListFree( type ) {                                                        \
            if (ptr->items)                                                                     \
                free( ptr->items );                                                             \
            ptr->items = NULL;                                                                  \
            ptr->num = 0;                                                                       \
            ptr->firstinactive = 0;                                                             \
        }
#undef ManagedListFreeName

/************************************************************************************************
    MANAGED LIST BATCH ALLOCATION
************************************************************************************************/

#define ManagedListAllocName _ManagedListAlloc_
#define ManagedListAlloc( type, ptr ) ManagedListAllocName##type( ptr )

#define DeclareManagedListAlloc( type )                                                         \
        void ManagedListAllocName##type( ManagedList( type )* ptr )

#define DefineManagedListAlloc( type )                                                          \
        DeclareManagedListAlloc( type ) {                                                       \
            size_t size = sizeof( type ) * ( ptr->num + ptr->batchsize );                       \
            type* resized = realloc( ptr->items, size );                                        \
            if ( resized ) {                                                                    \
                ptr->items = resized;                                                           \
                typeof(ptr->num) oldnum = ptr->num;                                             \
                ptr->num += ptr->batchsize;                                                     \
                typeof(ptr->num) i;                                                             \
                for ( i = oldnum; i < ptr->num; i++ ) {                                         \
                    ptr->items[i].active = 0;                                                   \
                }                                                                               \
            }                                                                                   \
            else {                                                                              \
                gamelog( "Ran out of memory allocating batch of type " #type );                 \
            }                                                                                   \
        }
#undef ManagedListAllocName

/************************************************************************************************
    MANAGED LIST FIRST INACTIVE RETRIEVAL
************************************************************************************************/

#define ManagedListUseFirstInactiveName _ManagedListUseFirstInactive_
#define ManagedListUseFirstInactive( type, ptr ) ManagedListUseFirstInactiveName##type( ptr )

#define DeclareManagedListUseFirstInactive( type )                                              \
        type* ManagedListUseFirstInactiveName##type( ManagedList( type )* ptr )                 \

#define DefineManagedListUseFirstInactive( type )                                               \
        DeclareManagedListUseFirstInactive( type ) {                                            \
            if ( ptr->num <= ptr->firstinactive ) {                                             \
                ManagedListAlloc( type, ptr );                                                  \
                type* result = &ptr->items[ptr->firstinactive];                                 \
                ptr->firstinactive++;                                                           \
                result->active = 1;                                                             \
                return result;                                                                  \
            }                                                                                   \
            else {                                                                              \
                type* result = &ptr->items[ptr->firstinactive];                                 \
                for ( ptr->firstinactive++; ptr->firstinactive < ptr->num;                      \
                      ptr->firstinactive++ ) {                                                  \
                    if ( !ptr->items[ptr->firstinactive].active ) {                             \
                        break;                                                                  \
                    }                                                                           \
                }                                                                               \
                                                                                                \
                if ( ptr->firstinactive == ptr->num ) {                                         \
                    ManagedListAlloc( type, ptr );                                              \
                }                                                                               \
                result->active = 1;                                                             \
                return result;                                                                  \
            }                                                                                   \
        }
#undef ManagedListUseFirstInactiveName

/************************************************************************************************
    MANAGED LIST ITEM DEACTIVATION
************************************************************************************************/

#define ManagedListDeactivateName _ManagedListDeactivate_
#define ManagedListDeactivate( type, ptr, index ) ManagedListDeactivateName##type( ptr, index )

#define DeclareManagedListDeactivate( type )                                                    \
        void ManagedListDeactivateName##type( ManagedList( type )* ptr, unsigned index )

#define DefineManagedListDeactivate( type )                                                     \
        DeclareManagedListDeactivate( type ) {                                                  \
            if ( index < 0 || index > ptr->num ) {                                              \
                gamelog( "Index to list of type " #type " is out of range. (%d)", index );      \
                return;                                                                         \
            }                                                                                   \
                                                                                                \
            ptr->items[index].active = 0;                                                       \
            if ( index < ptr->firstinactive ) {                                                 \
                ptr->firstinactive = index;                                                     \
            }                                                                                   \
        }
#undef ManagedListDeactivateName

#endif
