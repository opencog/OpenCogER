/*
  File: RPC_Client_Face_Emotion.hpp.in
  Date: November, 2018
  Author: Dagim Sisay <dagiopia@gmail.com>
  License: AGPL
*/

#ifndef _RPC_CLIENT_HPP_
#define _RPC_CLIENT_HPP_

#include <iostream>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "VisionCommon.hpp"
#include "util/base64.h"

#include "EmotionService.grpc.pb.h"

#define SERVER_ADDRESS "localhost:50051"
#define IMG_ENCODING ".jpg"

using grpc::Status;
using grpc::Channel;
using grpc::CreateChannel;
using grpc::ClientContext;


class RPC_Client_Face_Emotion
{    
    public:
      RPC_Client_Face_Emotion() : stub_(EmotionRecognition::NewStub(grpc::CreateChannel(
      				SERVER_ADDRESS, grpc::InsecureChannelCredentials()))) {}
      ~RPC_Client_Face_Emotion() {}

      bool detect_emotion(cv::Mat &in, std::vector<std::string> &out_emos, 
		                                 std::vector<cv::Rect> &out_boxes);

    private:
      std::unique_ptr<EmotionRecognition::Stub> stub_;

      Status status;
      std::vector<unsigned char> vbuff;
      unsigned char *ucbuff;
      cv::Rect rct;

      std::string encode_img_b64(cv::Mat);

};


#endif //_RPC_CLIENT_HPP_
