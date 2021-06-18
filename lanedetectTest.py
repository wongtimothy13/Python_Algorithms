import time
import math
import numpy as np
import cv2
# import rospy


img = cv2.imread(r'C:\Users\STP\Desktop\mp1_tester\test.png', 0)

def line_fit(binary_warped):
    """
    Find and fit lane lines
    """
    # print(binary_warped.shape)
    
    # cv2.imshow('temp', binary_warped)
    # cv2.waitKey(0)
    # cv2.destroyAllWindows()

    # Assuming you have created a warped binary image called "binary_warped"
    # Take a histogram of the bottom half of the image
    histogram = np.sum(binary_warped[binary_warped.shape[0]//2:,:], axis=0)
    # Create an output image to draw on and visualize the result
    out_img = (np.dstack((binary_warped, binary_warped, binary_warped))*255).astype('uint8')
    # Find the peak of the left and right halves of the histogram
    # These will be the starting point for the left and right lines
    midpoint = int(histogram.shape[0]/2)
    # print(f"Midpoint: {midpoint}")
    leftx_base = np.argmax(histogram[0:midpoint]) 
    rightx_base = np.argmax(histogram[midpoint:]) + midpoint
    

    # print(rightx_base)
    # Choose the number of sliding windows
    nwindows = 9
    # Set height of windows
    window_height = int(binary_warped.shape[0]/nwindows)
    # Identify the x and y positions of all nonzero pixels in the image
    nonzero = binary_warped.nonzero()
    nonzeroy = np.array(nonzero[0])
    nonzerox = np.array(nonzero[1])
    # Current positions to be updated for each window
    leftx_current = leftx_base
    rightx_current = rightx_base
    # Set the width of the windows +/- margin
    margin = 100
    # Set minimum number of pixels found to recenter window
    minpix = 50
    # Create empty lists to receive left and right lane pixel indices
    left_lane_inds = []
    right_lane_inds = []

    # print(f"Nonzers {len(nonzeroy)}")
    # Step through the windows one by one
    for window in range(nwindows-1, -1, -1):
        # print(window)
        # color = (255,0,0)
        # thickness = 2
        # Identify window boundaries in x and y (and right and left)
        ##TO DO
        # lstart = (int(leftx_current - margin), int(window * window_height))
        # lend = (int(leftx_current + margin), int((window+1) * window_height))
        # rstart = (int(rightx_current - margin), int(window * window_height))
        # rend = (int(rightx_current + margin), int((window+1) * window_height))
        ####
        # Draw the windows on the visualization image using cv2.rectangle()
        ##TO DO
        # out_img = cv2.rectangle(out_img, lstart, lend, color, thickness)
        # out_img = cv2.rectangle(out_img, rstart, rend, color, thickness)
        ####
        # Identify the nonzero pixels in x and y within the window
    
        win_y_low = window * window_height
        win_y_high = (window+1) * window_height
        win_xleft_low = leftx_current - margin
        win_xleft_high = leftx_current + margin
        win_xright_low = rightx_current - margin
        win_xright_high = rightx_current + margin

        # print(win_xright_low)
        # print(win_xright_high)
        # https://medium.com/typeiqs/advanced-lane-finding-c3c8305f074

        # calculate for the left side
        good_left_inds = ((nonzeroy >= win_y_low) & (nonzeroy < win_y_high) & (nonzerox >= win_xleft_low) &  (nonzerox < win_xleft_high)).nonzero()[0]
        # calculate for the right side
        good_right_inds = ((nonzeroy >= win_y_low) & (nonzeroy < win_y_high) & (nonzerox >= win_xright_low) &  (nonzerox < win_xright_high)).nonzero()[0]
        # print(f"{Lxtmp}, {Rxtmp}")
        ####
        # Append these indices to the lists
        ##TO DO
        left_lane_inds = np.append(left_lane_inds, good_left_inds)
        right_lane_inds = np.append(right_lane_inds, good_right_inds)
        left_lane_inds = left_lane_inds.astype(int)
        right_lane_inds = right_lane_inds.astype(int)
        ####
        # If you found > minpix pixels, recenter next window on their mean position
        ##TO DO
        if len(good_left_inds) > minpix:
            leftx_current = int(np.mean(nonzerox[good_left_inds]))
        if len(good_right_inds) > minpix:        
            rightx_current = int(np.mean(nonzerox[good_right_inds]))

        lcolor = (170,50,50)
        rcolor  = (80,50,170)
        thickness = 2
        lstart = (int(leftx_current-margin), int(window * window_height))
        lend = (int(leftx_current+margin), int((window + 1) * window_height))
        out_img = cv2.rectangle(out_img, lstart, lend, lcolor, thickness)
        rstart = (int(rightx_current - margin), int (window*window_height))
        rend = (int(rightx_current+margin), int((window+1)*window_height))
        out_img = cv2.rectangle(out_img, rstart, rend, rcolor, thickness)
    
    cv2.imshow('testwindow',out_img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
    

    # Concatenate the arrays of indices
    # left_lane_inds = np.concatenate(left_lane_inds)
    # right_lane_inds = np.concatenate(right_lane_inds)

    # Extract left and right line pixel positions
    leftx = nonzerox[left_lane_inds]
    lefty = nonzeroy[left_lane_inds] 
    rightx = nonzerox[right_lane_inds]
    righty = nonzeroy[right_lane_inds]

    print(f"{leftx}, {lefty}")

    # Fit a second order polynomial to each using np.polyfit()
    # If there isn't a good fit, meaning any of leftx, lefty, rightx, and righty are empty, 
    # the second order polynomial is unable to be sovled. 
    # Thus, it is unable to detect edges. 
    try:
    ##TODO
        left_fit = np.polyfit(lefty, leftx, 2)
        right_fit = np.polyfit(righty, rightx, 2)
        # print(left_fit)
        # print(right_fit)
    ####
    except TypeError:
        print("Unable to detect lanes")


    # Return a dict of relevant variables
    ret = {}
    ret['left_fit'] = left_fit
    ret['right_fit'] = right_fit
    ret['nonzerox'] = nonzerox
    ret['nonzeroy'] = nonzeroy
    ret['out_img'] = out_img
    ret['left_lane_inds'] = left_lane_inds
    ret['right_lane_inds'] = right_lane_inds

    return ret



line_fit(img)



