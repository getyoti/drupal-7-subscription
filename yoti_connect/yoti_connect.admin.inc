<?php

/**
 * @param $form
 * @param $form_state
 * @return mixed
 */
function yoti_connect_api_keys_settings($form, &$form_state)
{
    // show instructions to README.txt and Drupal account settings
    //  drupal_set_message(t('Installation and configuration instructions can be found from the README.txt'));
    //  drupal_set_message(t('Also check <a href="@url">Drupal account settings</a>', array('@url' => url('admin/config/people/accounts'))));

    // make sure private path exists, if not, create it
    $uploadDir = YotiConnectHelper::uploadDir(true);
    if ($uploadDir && !is_dir($uploadDir))
    {
        mkdir($uploadDir, 0777, true);
    }

    $form['some_text'] = array(
        '#markup' => '<div>' .
            t('You need to first create a Yoti App at <a href="@yoti-dev" target="_blank">@yoti-dev</a>.', array('@yoti-dev' => \Yoti\YotiClient::DASHBOARD_URL)) . '</br >' .
            t('Note: On the Yoti Dashboard the callback URL should be set to:') . ' <strong>' . url('/yoti-connect/link', array('absolute' => true)) . '</strong>' .
            '</div>',
    );

    $form['#attributes'] = array(
        'enctype' => "multipart/form-data",
    );

    $form['yoti_connect_settings']['yoti_app_id'] = array(
        '#type' => 'textfield',
        '#required' => true,
        '#title' => t('App ID'),
        '#default_value' => variable_get('yoti_app_id'),
        '#description' => t('Copy the App ID of your Yoti App here'),
    );

    $form['yoti_connect_settings']['yoti_sdk_id'] = array(
        '#type' => 'textfield',
        '#required' => true,
        '#title' => t('SDK ID'),
        '#default_value' => variable_get('yoti_sdk_id'),
        '#description' => t('Copy the SDK ID of your Yoti App here'),
    );

    $form['yoti_connect_settings']['yoti_pem'] = array(
        '#type' => 'managed_file',
        '#field_name' => 'yoti_pem',
        '#required' => true,
        '#title' => t('PEM File'),
        '#default_value' => variable_get('yoti_pem'),
        '#upload_location' => YotiConnectHelper::uploadDir(false),
        //        '#upload_location' => YotiConnectHelper::uploadDir(),
        '#description' => t('Upload the PEM file of your Yoti App here'),
        '#upload_validators' => array(
            'file_validate_extensions' => array('pem'),
            //        'file_validate_size' => array(25600000),
        ),
    );

    // Load the file.
    $pemFile = variable_get('yoti_pem');
    if ($pemFile)
    {
        $file = file_load($pemFile);
        // Change status to permanent.
        if (gettype($file) == 'object')
        {
            $file->status = FILE_STATUS_PERMANENT;
            // Save.
            file_save($file);
            //        $user = \Drupal\user\Entity\User::load(\Drupal::currentUser()->id());
            //        $file->setOwner($user);
            // Record the module (in this example, user module) is using the file.
            //        \Drupal::service('file.usage')->add($file, 'yoti_connect', 'yoti_connect', $file->id());
            file_usage_add($file, 'yoti_connect', 'yoti_pem', $file->fid);
            //      $_SESSION['intermedia'] = 'nothing';
            //drupal_set_message('File Saved');
        }
    }

    return system_settings_form($form);
}


function yoti_connect_list($form)
{
    $header = array(
        'email_address' => array('data' => 'Email Address', 'field' => 'y.email_address'),
        'phone_number' => array('data' => 'Phone', 'field' => 'y.phone_number'),
        'date_of_birth' => array('data' => 'Birthday', 'field' => 'y.date_of_birth'),
        'given_names' => array('data' => 'Given names', 'field' => 'y.given_names'),
        'family_name' => array('data' => 'Family name', 'field' => 'y.family_name'),
        'nationality' => array('data' => 'Nationality', 'field' => 'y.nationality'),
        'gender' => array('data' => 'Gender', 'field' => 'y.gender'),
    );

    $query = db_select('users_yoti', 'y');
    $query->condition('y.uid', 0, '<>');
    //    user_build_filter_query($query);

    $count_query = clone $query;
    $count_query->addExpression('COUNT(y.uid)');

    $query = $query->extend('PagerDefault')->extend('TableSort');
    $query->fields('y', array(
        'uid',
        'email_address',
        'phone_number',
        'date_of_birth',
        'given_names',
        'family_name',
        'nationality',
        'gender',
    ))->limit(50)
        ->orderByHeader($header)
        ->setCountQuery($count_query);
    $result = $query->execute();

    $options = array();
    foreach ($result as $account)
    {
        //        $users_roles = array();
        //        $roles_result = db_query('SELECT rid FROM {users_roles} WHERE uid = :uid', array(':uid' => $account->uid));
        //        foreach ($roles_result as $user_role)
        //        {
        //            $users_roles[] = $roles[$user_role->rid];
        //        }
        //        asort($users_roles);

        $options[$account->uid] = array(
            //            'title' => array(
            //                'data' => array(
            //                    '#type' => 'link',
            //                    '#title' => $account->email_address,
            //                    '#href' => 'node/' . $account->uid,
            //                    //                        '#options' => $l_options,
            //                    //                        '#suffix' => ' ' . theme('mark', array('type' => node_mark($node->nid, $node->changed))),
            //                ),
            //            ),
            'email_address' => $account->email_address,
            'phone_number' => $account->phone_number,
            'date_of_birth' => $account->date_of_birth,
            'given_names' => $account->given_names,
            'family_name' => $account->family_name,
            'nationality' => $account->nationality,
            'gender' => $account->gender,
            //            'username' => theme('username', array('account' => $account)),
            //            'status' => $status[$account->status],
            //            'roles' => theme('item_list', array('items' => $users_roles)),
            //            'member_for' => format_interval(REQUEST_TIME - $account->created),
            //            'access' => $account->access ? t('@time ago', array('@time' => format_interval(REQUEST_TIME - $account->access))) : t('never'),
            //            'operations' => array(
            //                'data' => array(
            //                    '#type' => 'link',
            //                    '#title' => t('edit'),
            //                    '#href' => "user/$account->uid/edit",
            //                    '#options' => array('query' => $destination),
            //                ),
            //            ),
        );
    }

    // export csv
    if (!empty($_POST['yoti-connect-csv']))
    {
        $filename = date("Y-m-d") . "-yoti-users.csv";

        $cols = array(
            'email_address' => 'Email',
            'phone_number' => 'Phone',
            'date_of_birth' => 'DOB',
            'given_names' => 'Given Names',
            'family_name' => 'Family Name',
            'nationality' => 'Nationality',
            'gender' => 'Gender',
        );

        // output headers so that the file is downloaded rather than displayed
        header('Content-Type: text/csv; charset=utf-8');
        header("Content-Disposition: attachment; filename=$filename");

        // create a file pointer connected to the output stream
        $output = fopen('php://output', 'w');
        fputcsv($output, array_values($cols));
        foreach ($options as $option)
        {
            $row = array_intersect_key($option, $cols);
            fputcsv($output, array_values($row));
        }
        exit;
    }

    if (!empty($_POST['yoti-connect-delete']))
    {
        $ids = !empty($_POST['accounts']) ? $_POST['accounts'] : [];
        if ($ids)
        {
            $query = db_delete('users_yoti');
            $query->condition('uid', $ids, 'IN');
            $query->execute();
        }

        drupal_goto('admin/people/yoti-connect-list');
    }

    $form['options']['export'] = array(
        '#type' => 'submit',
        '#name' => 'yoti-connect-csv',
        '#value' => t('Export CSV'),
    );

    $form['options']['delete'] = array(
        '#type' => 'submit',
        '#name' => 'yoti-connect-delete',
        '#value' => t('Delete checked'),
    );

    $form['accounts'] = array(
        '#type' => 'tableselect',
        //        '#attributes' => null,
        '#header' => $header,
        '#options' => $options,
        '#empty' => t('No people available.'),
        '#attributes' => array('class' => array('yoti-connect-list')),
        '#multiple' => true,
    );
    //    $form['pager'] = array('#markup' => theme('pager'));

    return $form;
}

//function that calls your form
function yoti_connect_list_view()
{
    $form = drupal_get_form('yoti_connect_list');

    return $form;
}