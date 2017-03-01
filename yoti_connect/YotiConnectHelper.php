<?php
use Yoti\ActivityDetails;
use Yoti\YotiClient;

require_once __DIR__ . '/sdk/boot.php';


/**
 * Class YotiConnectHelper
 *
 * @package Drupal\yoti_connect
 *
 */
class YotiConnectHelper
{
    /**
     * @var array
     */
    public static $profileFields = array(
        ActivityDetails::ATTR_SELFIE => 'Selfie',
        ActivityDetails::ATTR_PHONE_NUMBER => 'Phone number',
        ActivityDetails::ATTR_DATE_OF_BIRTH => 'Date of birth',
        ActivityDetails::ATTR_GIVEN_NAMES => 'Given names',
        ActivityDetails::ATTR_FAMILY_NAME => 'Family name',
        ActivityDetails::ATTR_NATIONALITY => 'Nationality',
    );

    /**
     * Running mock requests instead of going to yoti
     * @return bool
     */
    public static function mockRequests()
    {
        return defined('YOTI_MOCK_REQUEST') && YOTI_MOCK_REQUEST;
    }

    /**
     * @return bool
     */
    public function link()
    {
        $config = self::getConfig();
        $token = (!empty($_GET['token'])) ? $_GET['token'] : null;

        // if no token then ignore
        if (!$token)
        {
            $this->setFlash('Could not get Yoti token.', 'error');

            return false;
        }

        // init yoti client and attempt to request user details
        try
        {
            $yotiClient = new YotiClient($config['yoti_sdk_id'], $config['yoti_pem']['contents']);
            $yotiClient->setMockRequests(self::mockRequests());
            $activityDetails = $yotiClient->getActivityDetails($token);
        }
        catch (Exception $e)
        {
            $this->setFlash('Yoti could not successfully connect to your account.', 'error');

            return false;
        }

        // if unsuccessful then bail
        if ($yotiClient->getOutcome() != YotiClient::OUTCOME_SUCCESS)
        {
            $this->setFlash('Yoti could not successfully connect to your account.', 'error');

            return false;
        }

        // check if yoti user exists
        $exists = $this->getUserIdByYotiId($activityDetails->getUserId());
        if ($exists)
        {
            $this->setFlash('Yoti user already stored in database.', 'error');
        }
        else
        {
            $this->createYotiUser($activityDetails);
            $this->setFlash('Yoti account stored successfully.');
        }

        return true;
    }

    /**
     * @param $message
     * @param string $type
     */
    private function setFlash($message, $type = 'status')
    {
        drupal_set_message($message, $type);
    }

    /**
     * @param $yotiId
     * @return int
     */
    private function getUserIdByYotiId($yotiId)
    {
        $tableName = self::tableName();
        $col = db_query("SELECT identifier FROM `{$tableName}` WHERE identifier = '$yotiId'")->fetchCol();
        return ($col) ? reset($col) : null;
    }

    /**
     * @param ActivityDetails $activityDetails
     */
    private function createYotiUser(ActivityDetails $activityDetails)
    {
        //        $user = user_load($userId);
        $meta = $activityDetails->getProfileAttribute();
        unset($meta[ActivityDetails::ATTR_SELFIE]); // don't save selfie to db

        $selfieFilename = null;
        if (($content = $activityDetails->getProfileAttribute(ActivityDetails::ATTR_SELFIE)))
        {
            $uploadDir = self::uploadDir();
            if (!is_dir($uploadDir))
            {
                drupal_mkdir($uploadDir, 0777, true);
            }

            $selfieFilename = md5("selfie" . time()) . ".png";
            file_put_contents("$uploadDir/$selfieFilename", $content);
            //      file_put_contents(self::uploadDir() . "/$selfieFilename", $activityDetails->getUserProfile('selfie'));
            $meta['selfie_filename'] = $selfieFilename;
        }

        //        foreach (self::$profileFields as $param => $label)
        //        {
        //            $meta[$param] = $activityDetails->getProfileAttribute($param);
        //        }
        //unset($meta[ActivityDetails::ATTR_SELFIE]); // don't save selfie to db

        db_insert(self::tableName())->fields(array(
//            'uid' => $userId,
            'identifier' => $activityDetails->getUserId(),
            'phone_number' => $activityDetails->getProfileAttribute(ActivityDetails::ATTR_PHONE_NUMBER),
            'date_of_birth' => $activityDetails->getProfileAttribute(ActivityDetails::ATTR_DATE_OF_BIRTH),
            'given_names' => $activityDetails->getProfileAttribute(ActivityDetails::ATTR_GIVEN_NAMES),
            'family_name' => $activityDetails->getProfileAttribute(ActivityDetails::ATTR_FAMILY_NAME),
            'nationality' => $activityDetails->getProfileAttribute(ActivityDetails::ATTR_NATIONALITY),
            'gender' => $activityDetails->getProfileAttribute('gender'),
            'email_address' => $activityDetails->getProfileAttribute('email_address'),
            'selfie_filename' => $selfieFilename,
            'data' => serialize($meta),
        ))->execute();
    }

    /**
     * not used in this instance
     * @return string
     */
    public static function tableName()
    {
        return 'users_yoti_civi';
    }

    /**
     * @param bool $realPath
     * @return string
     */
    public static function uploadDir($realPath = true)
    {
        return ($realPath) ? drupal_realpath("yoti://") : 'yoti://';
    }

    /**
     * @return string
     */
    public static function uploadUrl()
    {
        return file_create_url(self::uploadDir());
    }

    /**
     * @return array
     */
    public static function getConfig()
    {
        if (self::mockRequests())
        {
            $config = require_once __DIR__ . '/sdk/sample-data/config.php';
            return $config;
        }

        $pem = variable_get('yoti_pem');
        $name = $contents = null;
        if ($pem)
        {
            $file = file_load($pem);
            $name = $file->uri;
            $contents = file_get_contents(drupal_realpath($name));
        }
        return array(
            'yoti_app_id' => variable_get('yoti_app_id'),
            'yoti_sdk_id' => variable_get('yoti_sdk_id'),
            'yoti_pem' => array(
                'name' => $name,
                'contents' => $contents,
            ),
        );
    }

    /**
     * @return null|string
     */
    public static function getLoginUrl()
    {
        $config = self::getConfig();
        if (empty($config['yoti_app_id']))
        {
            return null;
        }

        //https://staging0.www.yoti.com/connect/ad725294-be3a-4688-a26e-f6b2cc60fe70
        //https://staging0.www.yoti.com/connect/990a3996-5762-4e8a-aa64-cb406fdb0e68

        return YotiClient::getLoginUrl($config['yoti_app_id']);
    }
}