/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <omx_base.h>
#include <OMX_TI_IVCommon.h>
#include <OMX_TI_Video.h>
//#define OMX_SAMPLE_EXTRADATA_TEST

#define TOOLS_LOG_BUFFER_SIZE_BYTES 128

#ifdef OMX_SAMPLE_EXTRADATA_TEST
/*This is only for testing extra data by sample comonent, no relevance should be
attached to these lines*/
 #define OMX_ExtraDataSAMPLEA OMX_ExtraDataKhronosExtensions
 #define OMX_ExtraDataSAMPLEB OMX_ExtraDataVendorStartUnused
#endif
/*===============================================================*/
/** OMX_BASE_ExtraDataTableType  : This table contains details of the various
 *                                 extra data types. Derived components can
 *                                 update this table to add new extra data types
 *                                 Any new extra data types also need to be
 *                                 to be added in the OMX_EXTRADATATYPE
 *                                 enumeration after
 *                                 OMX_ExtraDataVendorStartUnused. This is
 *                                 defined in omx_base.h. The table is
 *                                 maintained below.
 *
 *  @ param eExtraDataType       : This is the extra data type from the
 *                                 OMX_EXTRADATATYPE enumeration.
 *  @ param nSize                : This is the size of the extra data type in
 *                                 bytes. If extra data is of constant
 *                                 size (e.g. a 4 byte integer) then that size
 *                                 is added here. If the extra data consists of
 *                                 packets of data of variable size then the
 *                                 size will be 8 (in the header there will be
 *                                 4 bytes for the pointer to the data and 4
 *                                 bytes for the size of the size of the data)
 *  @ param bPacketData          : Whether the extra data consists of variable
 *                                 sized packets of data. If true then size
 *                                 should be set to 8 bytes.
 */
/*===============================================================*/

/*This table lists the various extra data types used by derived components and
their properties. Any update to this table should be matched by a corresponding
update to the OMX_EXTRADATATYPE enumeration. Details on the on the various
of OMX_BASE_ExtraDataTableType are provided above.*/
OMX_BASE_EXTRA_DATA_TABLE    OMX_BASE_ExtraDataTable[] =
{
    { OMX_ExtraDataQuantization, 8, OMX_TRUE },
#ifdef OMX_SAMPLE_EXTRADATA_TEST
    /*These are not actual extra data types but only samples to be used by sample
    component for testing*/
    { OMX_ExtraDataSAMPLEA, 4, OMX_FALSE },
    { OMX_ExtraDataSAMPLEB, 4, OMX_FALSE },
#endif
    { (OMX_EXTRADATATYPE) OMX_AncillaryData, 1024 * 82, OMX_FALSE },
    { (OMX_EXTRADATATYPE) OMX_TI_FrameLayout, 8, OMX_FALSE },
    { (OMX_EXTRADATATYPE) OMX_TI_SEIinfo2004Frame1, sizeof(OMX_TI_STEREODECINFO), OMX_FALSE },
    { (OMX_EXTRADATATYPE) OMX_TI_SEIinfo2004Frame2, sizeof(OMX_TI_STEREODECINFO), OMX_FALSE },
    { (OMX_EXTRADATATYPE) OMX_TI_SEIinfo2010Frame1, sizeof(OMX_TI_FRAMEPACKINGDECINFO), OMX_FALSE },
    { (OMX_EXTRADATATYPE) OMX_TI_SEIinfo2010Frame2, sizeof(OMX_TI_FRAMEPACKINGDECINFO), OMX_FALSE },
    { (OMX_EXTRADATATYPE) OMX_TI_RangeMappingInfo, sizeof(OMX_TI_VIDEO_RANGEMAPPING), OMX_FALSE },
    { (OMX_EXTRADATATYPE) OMX_TI_RescalingInfo, sizeof(OMX_TI_VIDEO_RESCALINGMATRIX), OMX_FALSE },
    { (OMX_EXTRADATATYPE) OMX_TI_H264ESliceDataInfo, sizeof(OMX_TI_VIDEO_SLICEDATAINFO), OMX_FALSE },
    { (OMX_EXTRADATATYPE) OMX_TI_ProfilerData, TOOLS_LOG_BUFFER_SIZE_BYTES, OMX_FALSE },
    { (OMX_EXTRADATATYPE) OMX_TI_LSCTable, sizeof(OMX_TI_LSCTABLETYPE), OMX_FALSE },
    { (OMX_EXTRADATATYPE) OMX_TI_ProfilerData, TOOLS_LOG_BUFFER_SIZE_BYTES, OMX_FALSE },
    { (OMX_EXTRADATATYPE) OMX_TI_ExtraData_AFStatistics, sizeof(OMX_TI_AF_STATISTICS_TYPE), OMX_FALSE},
    { (OMX_EXTRADATATYPE) OMX_TI_ExtraData_AEWBStatistics, sizeof(OMX_TI_AEWB_STATISTICS_TYPE), OMX_FALSE},
    { (OMX_EXTRADATATYPE) OMX_TI_ExtraData_BSCStatistics, sizeof(OMX_TI_BSC_STATISTICS_TYPE), OMX_FALSE},
    { (OMX_EXTRADATATYPE) OMX_TI_ExtraData_AuxiliaryImage, sizeof(OMX_TI_AUX_IMAGEDATA_TYPE), OMX_FALSE},
    { (OMX_EXTRADATATYPE) OMX_TI_CodecExtenderErrorFrame1, 4, OMX_FALSE },
    { (OMX_EXTRADATATYPE) OMX_TI_CodecExtenderErrorFrame2, 4, OMX_FALSE },
    { (OMX_EXTRADATATYPE) OMX_TI_MBInfoFrame1, 8, OMX_TRUE },
    { (OMX_EXTRADATATYPE) OMX_TI_MBInfoFrame2, 8, OMX_TRUE },
    { (OMX_EXTRADATATYPE) OMX_TI_SEIInfoFrame1, 8, OMX_TRUE },
    { (OMX_EXTRADATATYPE) OMX_TI_SEIInfoFrame2, 8, OMX_TRUE },
    { (OMX_EXTRADATATYPE) OMX_TI_VUIInfoFrame1, 8, OMX_TRUE },
    { (OMX_EXTRADATATYPE) OMX_TI_VUIInfoFrame2, 8, OMX_TRUE },
    { (OMX_EXTRADATATYPE) OMX_FaceDetection, sizeof(OMX_FACEDETECTIONTYPE), OMX_FALSE },
    { (OMX_EXTRADATATYPE)0, 0, (OMX_BOOL)0 }
};
//OMX_AncillaryData

