#pragma once

#include "partition_id.h"

#include <ydb/core/base/events.h>
#include <ydb/core/grpc_services/rpc_calls.h>
#include <ydb/core/protos/pqconfig.pb.h>
#include <ydb/core/persqueue/percentile_counter.h>

#include <ydb/public/api/protos/persqueue_error_codes_v1.pb.h>

#include <ydb/services/lib/actors/type_definitions.h>

#include <util/generic/guid.h>


namespace NKikimr::NGRpcProxy::V1 {

using namespace Ydb;

// unused?
// struct TCommitCookie {
//     ui64 AssignId;
//     ui64 Cookie;
// };

struct TEvPQProxy {
    enum EEv {
        EvWriteInit = EventSpaceBegin(TKikimrEvents::ES_PQ_PROXY_NEW), // TODO: Replace 'NEW' with version or something
        EvTopicWriteInit,
        EvWrite,
        EvTopicWrite,
        EvDone,
        EvReadInit,
        EvMigrationReadInit,
        EvRead,
        EvCloseSession,
        EvPartitionReady,
        EvReadResponse,
        EvMigrationReadResponse,
        EvCommitCookie,
        EvCommitDone,
        EvStartRead,
        EvReleasePartition,
        EvReleased,
        EvPartitionReleased,
        EvLockPartition,
        EvRestartPipe,
        EvDieCommand,
        EvPartitionStatus,
        EvAuth,
        EvReadSessionStatus,
        EvReadSessionStatusResponse,
        EvAuthResultOk,
        EvUpdateClusters,
        EvQueryCompiled,
        EvSessionDead,
        EvSessionSetPreferredCluster,
        EvScheduleUpdateClusters,
        EvDeadlineExceeded,
        EvGetStatus,
        EvUpdateToken,
        EvTopicUpdateToken,
        EvCommitRange,
        EvRequestTablet,
        EvEnd
    };


    struct TEvReadSessionStatus : public TEventPB<TEvReadSessionStatus, NKikimrPQ::TReadSessionStatus, EvReadSessionStatus> {
    };

    struct TEvReadSessionStatusResponse : public TEventPB<TEvReadSessionStatusResponse, NKikimrPQ::TReadSessionStatusResponse, EvReadSessionStatusResponse> {
    };


    struct TEvAuthResultOk : public NActors::TEventLocal<TEvAuthResultOk, EvAuthResultOk> {
        TEvAuthResultOk(const TTopicInitInfoMap&& topicAndTablets)
            : TopicAndTablets(std::move(topicAndTablets))
        { }

        TTopicInitInfoMap TopicAndTablets;
    };

    struct TEvSessionSetPreferredCluster : public NActors::TEventLocal<TEvSessionSetPreferredCluster, EvSessionSetPreferredCluster> {
        TEvSessionSetPreferredCluster(const ui64 cookie, const TString& preferredCluster)
            : Cookie(cookie)
            , PreferredCluster(preferredCluster)
        {}
        const ui64 Cookie;
        const TString PreferredCluster;
    };

    struct TEvSessionDead : public NActors::TEventLocal<TEvSessionDead, EvSessionDead> {
        TEvSessionDead(const ui64 cookie)
            : Cookie(cookie)
        { }

        const ui64 Cookie;
    };

    struct TEvScheduleUpdateClusters : public NActors::TEventLocal<TEvScheduleUpdateClusters, EvScheduleUpdateClusters> {
        TEvScheduleUpdateClusters()
        { }
    };


    struct TEvUpdateClusters : public NActors::TEventLocal<TEvUpdateClusters, EvUpdateClusters> {
        TEvUpdateClusters(const TString& localCluster, bool enabled, const TVector<TString>& clusters)
            : LocalCluster(localCluster)
            , Enabled(enabled)
            , Clusters(clusters)
        { }

        const TString LocalCluster;
        const bool Enabled;
        const TVector<TString> Clusters;
    };

    struct TEvQueryCompiled : public NActors::TEventLocal<TEvQueryCompiled, EvQueryCompiled> {
        TEvQueryCompiled(const TString& selectQ, const TString& updateQ, const TString& deleteQ)
            : SelectQ(selectQ)
            , UpdateQ(updateQ)
            , DeleteQ(deleteQ)
        { }

        const TString SelectQ, UpdateQ, DeleteQ;
    };

    struct TEvWriteInit : public NActors::TEventLocal<TEvWriteInit, EvWriteInit> {
        TEvWriteInit(PersQueue::V1::StreamingWriteClientMessage&& req, const TString& peerName)
            : Request(std::move(req))
            , PeerName(peerName)
        { }

        PersQueue::V1::StreamingWriteClientMessage Request;
        TString PeerName;
    };

    struct TEvTopicWriteInit : public NActors::TEventLocal<TEvTopicWriteInit, EvTopicWriteInit> {
        TEvTopicWriteInit(Topic::StreamWriteMessage::FromClient&& req, const TString& peerName)
            : Request(std::move(req))
            , PeerName(peerName)
        { }

        Topic::StreamWriteMessage::FromClient Request;
        TString PeerName;
    };

    struct TEvWrite : public NActors::TEventLocal<TEvWrite, EvWrite> {
        explicit TEvWrite(PersQueue::V1::StreamingWriteClientMessage&& req)
            : Request(std::move(req))
        { }

        PersQueue::V1::StreamingWriteClientMessage Request;
    };

    struct TEvTopicWrite : public NActors::TEventLocal<TEvTopicWrite, EvTopicWrite> {
        explicit TEvTopicWrite(Topic::StreamWriteMessage::FromClient&& req)
            : Request(std::move(req))
        { }

        Topic::StreamWriteMessage::FromClient Request;
    };

    struct TEvDone : public NActors::TEventLocal<TEvDone, EvDone> {
        TEvDone()
        { }
    };

    struct TEvReadInit : public NActors::TEventLocal<TEvReadInit, EvReadInit> {
        TEvReadInit(const Topic::StreamReadMessage::FromClient& req, const TString& peerName)
            : Request(req)
            , PeerName(peerName)
        { }

        Topic::StreamReadMessage::FromClient Request;
        TString PeerName;
    };

    struct TEvMigrationReadInit : public NActors::TEventLocal<TEvMigrationReadInit, EvMigrationReadInit> {
        TEvMigrationReadInit(const PersQueue::V1::MigrationStreamingReadClientMessage& req, const TString& peerName)
            : Request(req)
            , PeerName(peerName)
        { }

        PersQueue::V1::MigrationStreamingReadClientMessage Request;
        TString PeerName;
    };

    struct TEvRead : public NActors::TEventLocal<TEvRead, EvRead> {
        explicit TEvRead(const TString& guid = CreateGuidAsString(), ui64 maxCount = 0, ui64 maxSize = 0, ui64 maxTimeLagMs = 0, ui64 readTimestampMs = 0)
            : Guid(guid)
            , MaxCount(maxCount)
            , MaxSize(maxSize)
            , MaxTimeLagMs(maxTimeLagMs)
            , ReadTimestampMs(readTimestampMs)
        { }

        explicit TEvRead(ui64 maxSize)
            : Guid(CreateGuidAsString())
            , MaxCount(0)
            , MaxSize(maxSize)
            , MaxTimeLagMs(0)
            , ReadTimestampMs(0) {
        }

        const TString Guid;
        ui64 MaxCount;
        ui64 MaxSize;
        ui64 MaxTimeLagMs;
        ui64 ReadTimestampMs;
    };

    struct TEvUpdateToken : public NActors::TEventLocal<TEvUpdateToken, EvUpdateToken> {
        explicit TEvUpdateToken(PersQueue::V1::StreamingWriteClientMessage&& req)
            : Request(std::move(req))
        { }

        PersQueue::V1::StreamingWriteClientMessage Request;
    };

    struct TEvTopicUpdateToken : public NActors::TEventLocal<TEvTopicUpdateToken, EvTopicUpdateToken> {
        explicit TEvTopicUpdateToken(Topic::StreamWriteMessage::FromClient&& req)
            : Request(std::move(req))
        { }

        Topic::StreamWriteMessage::FromClient Request;
    };

    struct TEvCloseSession : public NActors::TEventLocal<TEvCloseSession, EvCloseSession> {
        TEvCloseSession(const TString& reason, const PersQueue::ErrorCode::ErrorCode errorCode)
            : Reason(reason)
            , ErrorCode(errorCode)
        { }

        const TString Reason;
        PersQueue::ErrorCode::ErrorCode ErrorCode;
    };

    struct TEvPartitionReady : public NActors::TEventLocal<TEvPartitionReady, EvPartitionReady> {
        TEvPartitionReady(const TPartitionId& partition, const ui64 wTime, const ui64 sizeLag,
                          const ui64 readOffset, const ui64 endOffset)
            : Partition(partition)
            , WTime(wTime)
            , SizeLag(sizeLag)
            , ReadOffset(readOffset)
            , EndOffset(endOffset)
        { }

        const TPartitionId Partition;
        ui64 WTime;
        ui64 SizeLag;
        ui64 ReadOffset;
        ui64 EndOffset;
    };

    struct TEvReadResponse : public NActors::TEventLocal<TEvReadResponse, EvReadResponse> {
        explicit TEvReadResponse(Topic::StreamReadMessage::FromServer&& resp, ui64 nextReadOffset, bool fromDisk, TDuration waitQuotaTime)
            : Response(std::move(resp))
            , NextReadOffset(nextReadOffset)
            , FromDisk(fromDisk)
            , WaitQuotaTime(waitQuotaTime)
        { }

        Topic::StreamReadMessage::FromServer Response;
        ui64 NextReadOffset;
        bool FromDisk;
        TDuration WaitQuotaTime;
    };

    struct TEvMigrationReadResponse : public NActors::TEventLocal<TEvMigrationReadResponse, EvMigrationReadResponse> {
        explicit TEvMigrationReadResponse(PersQueue::V1::MigrationStreamingReadServerMessage&& resp, ui64 nextReadOffset, bool fromDisk, TDuration waitQuotaTime)
            : Response(std::move(resp))
            , NextReadOffset(nextReadOffset)
            , FromDisk(fromDisk)
            , WaitQuotaTime(waitQuotaTime)
        { }

        PersQueue::V1::MigrationStreamingReadServerMessage Response;
        ui64 NextReadOffset;
        bool FromDisk;
        TDuration WaitQuotaTime;
    };

    struct TCommitCookie {
        TVector<ui64> Cookies;
    };

    struct TCommitRange {
        TVector<std::pair<ui64, ui64>> Ranges;
    };

    struct TEvCommitCookie : public NActors::TEventLocal<TEvCommitCookie, EvCommitCookie> {
        explicit TEvCommitCookie(const ui64 assignId, const TCommitCookie&& commitInfo)
            : AssignId(assignId)
            , CommitInfo(std::move(commitInfo))
        { }

        ui64 AssignId;
        TCommitCookie CommitInfo;
    };

    struct TEvCommitRange : public NActors::TEventLocal<TEvCommitRange, EvCommitRange> {
        explicit TEvCommitRange(const ui64 assignId, const TCommitRange&& commitInfo)
            : AssignId(assignId)
            , CommitInfo(std::move(commitInfo))
        { }

        ui64 AssignId;
        TCommitRange CommitInfo;
    };




    struct TEvAuth : public NActors::TEventLocal<TEvAuth, EvAuth> {
        TEvAuth(const TString& auth)
            : Auth(auth)
        { }

        TString Auth;
    };

    struct TEvStartRead : public NActors::TEventLocal<TEvStartRead, EvStartRead> {
        TEvStartRead(ui64 id, ui64 readOffset, ui64 commitOffset, bool verifyReadOffset)
            : AssignId(id)
            , ReadOffset(readOffset)
            , CommitOffset(commitOffset)
            , VerifyReadOffset(verifyReadOffset)
            , Generation(0)
        { }

        const ui64 AssignId;
        ui64 ReadOffset;
        ui64 CommitOffset;
        bool VerifyReadOffset;
        ui64 Generation;
    };

    struct TEvReleased : public NActors::TEventLocal<TEvReleased, EvReleased> {
        TEvReleased(ui64 id)
            : AssignId(id)
        { }

        const ui64 AssignId;
    };

    struct TEvGetStatus : public NActors::TEventLocal<TEvGetStatus, EvGetStatus> {
        TEvGetStatus(ui64 id)
            : AssignId(id)
        { }

        const ui64 AssignId;
    };


    struct TEvCommitDone : public NActors::TEventLocal<TEvCommitDone, EvCommitDone> {
        explicit TEvCommitDone(const ui64 assignId, const ui64 startCookie, const ui64 lastCookie, const ui64 offset)
            : AssignId(assignId)
            , StartCookie(startCookie)
            , LastCookie(lastCookie)
            , Offset(offset)
        { }

        ui64 AssignId;
        ui64 StartCookie;
        ui64 LastCookie;
        ui64 Offset;
    };

    struct TEvReleasePartition : public NActors::TEventLocal<TEvReleasePartition, EvReleasePartition> {
        TEvReleasePartition()
        { }
    };

    struct TEvLockPartition : public NActors::TEventLocal<TEvLockPartition, EvLockPartition> {
        explicit TEvLockPartition(const ui64 readOffset, const ui64 commitOffset, bool verifyReadOffset, bool startReading)
            : ReadOffset(readOffset)
            , CommitOffset(commitOffset)
            , VerifyReadOffset(verifyReadOffset)
            , StartReading(startReading)
        { }

        ui64 ReadOffset;
        ui64 CommitOffset;
        bool VerifyReadOffset;
        bool StartReading;
    };


    struct TEvPartitionReleased : public NActors::TEventLocal<TEvPartitionReleased, EvPartitionReleased> {
        TEvPartitionReleased(const TPartitionId& partition)
            : Partition(partition)
        { }
        TPartitionId Partition;
    };


    struct TEvRestartPipe : public NActors::TEventLocal<TEvRestartPipe, EvRestartPipe> {
        TEvRestartPipe()
        { }
    };

    struct TEvDeadlineExceeded : public NActors::TEventLocal<TEvDeadlineExceeded, EvDeadlineExceeded> {
        TEvDeadlineExceeded(ui64 cookie)
            : Cookie(cookie)
        { }

        ui64 Cookie;
    };


    struct TEvDieCommand : public NActors::TEventLocal<TEvDieCommand, EvDieCommand> {
        TEvDieCommand(const TString& reason, const PersQueue::ErrorCode::ErrorCode errorCode)
            : Reason(reason)
            , ErrorCode(errorCode)
        { }

        TString Reason;
        PersQueue::ErrorCode::ErrorCode ErrorCode;
    };

    struct TEvPartitionStatus : public NActors::TEventLocal<TEvPartitionStatus, EvPartitionStatus> {
        TEvPartitionStatus(const TPartitionId& partition, const ui64 offset, const ui64 endOffset, const ui64 writeTimestampEstimateMs, bool init = true)
            : Partition(partition)
            , Offset(offset)
            , EndOffset(endOffset)
            , WriteTimestampEstimateMs(writeTimestampEstimateMs)
            , Init(init)
        { }

        TPartitionId Partition;
        ui64 Offset;
        ui64 EndOffset;
        ui64 WriteTimestampEstimateMs;
        bool Init;
    };
    struct TEvRequestTablet : public NActors::TEventLocal<TEvRequestTablet, EvRequestTablet> {
        TEvRequestTablet(const ui64 tabletId)
            : TabletId(tabletId)
        { }

        ui64 TabletId;
    };

};
}
