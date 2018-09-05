#ifndef STORAGEUTILS_HPP
#define STORAGEUTILS_HPP
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/Bucket.h>
#include <aws/s3/model/PutObjectRequest.h>

#include <boost/interprocess/streams/bufferstream.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class StorageUtils {
private:
  Aws::String endpoint;
  Aws::String accessId;
  Aws::String secretKey;
  Aws::Vector<Aws::S3::Model::Bucket> bucketList;

public:
  StorageUtils();
  inline Aws::Vector<Aws::S3::Model::Bucket> getListBucket() {
    return bucketList;
  }

  void uploadFile(cv::Mat &img, std::string fileName, std::string camid);
};

#endif // STORAGEUTILS_HPP
