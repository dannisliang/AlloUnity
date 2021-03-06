﻿using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System;
using System.Reflection;



public class RenderCubemap : MonoBehaviour {
	
	// Parameters
	public int resolution = 1024;
	public int faceCount  = 6;
	public bool extract   = true;
	
	[DllImport("CubemapExtractionPlugin")]
	private static extern void StartFromUnity(System.IntPtr[] texturePtrs, int cubemapFacesCount, int resolution);
	[DllImport("CubemapExtractionPlugin")]
	private static extern void StopFromUnity();
	
	private static System.String[] cubemapFaceNames = {
		"LeftEye/PositiveX",
		"LeftEye/NegativeX",
		"LeftEye/PositiveY",
		"LeftEye/NegativeY",
		"LeftEye/PositiveZ",
		"LeftEye/NegativeZ",
		"RightEye/PositiveX",
		"RightEye/NegativeX",
		"RightEye/PositiveY",
		"RightEye/NegativeY",
		"RightEye/PositiveZ",
		"RightEye/NegativeZ"
	};
	
	private static Vector3[] cubemapFaceRotations = {
		new Vector3(  0,  90, 0),
		new Vector3(  0, 270, 0),
		new Vector3(270,   0, 0),
		new Vector3( 90,   0, 0),
		new Vector3(  0,   0, 0),
		new Vector3(  0, 180, 0),
	};
	
	// Use this for initialization
	IEnumerator Start() {
		
		// Set up 6 cameras for cubemap
		Camera thisCam = GetComponent<Camera>();
		GameObject cubemap = new GameObject("Cubemap");
		cubemap.transform.parent = transform;
		
		// Move the cubemap to the origin of the parent cam
		cubemap.transform.localPosition = Vector3.zero;
		
		System.IntPtr[] texturePtrs = new System.IntPtr[faceCount];
		
		for (int i = 0; i < faceCount; i++)
		{
			GameObject go = new GameObject(cubemapFaceNames[i]);
			Camera cam = go.AddComponent<Camera>();
			
			// Set render texture
			RenderTexture tex = new RenderTexture(resolution, resolution, 1, RenderTextureFormat.ARGB32);
			tex.Create();
			cam.targetTexture = tex;
			cam.aspect = 1;
			
			// Set orientation
			cam.fieldOfView = 90;
			go.transform.eulerAngles = cubemapFaceRotations[i];
			go.transform.parent = cubemap.transform;
			
			// Move the cubemap to the origin of the parent cam
			cam.transform.localPosition = Vector3.zero;
			
			texturePtrs[i] = tex.GetNativeTexturePtr();
		}
		
		// Tell native plugin that rendering has started
		StartFromUnity(texturePtrs, faceCount, resolution);
		
		yield return StartCoroutine("CallPluginAtEndOfFrames");
	}
	
	void OnDestroy()
	{
		StopFromUnity();
	}
	
	
	private IEnumerator CallPluginAtEndOfFrames()
	{
		while (true)
		{
			yield return new WaitForEndOfFrame();
			if (extract)
			{
				GL.IssuePluginEvent(1);
			}
		}
	}
}