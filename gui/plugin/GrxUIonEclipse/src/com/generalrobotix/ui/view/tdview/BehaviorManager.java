/**
 * BehaviorManager.java
 *
 * @author  Kernel, Inc.
 * @version  2.0 (Mon Nov 12 2001)
 */

package com.generalrobotix.ui.view.tdview;

import java.util.List;

import javax.media.j3d.BranchGroup;
import javax.media.j3d.Transform3D;
import javax.media.j3d.TransformGroup;

import jp.go.aist.hrp.simulator.DynamicsSimulator;
import jp.go.aist.hrp.simulator.DynamicsSimulatorFactory;
import jp.go.aist.hrp.simulator.DynamicsSimulatorFactoryHelper;
import jp.go.aist.hrp.simulator.DynamicsSimulatorPackage.IntegrateMethod;
import jp.go.aist.hrp.simulator.DynamicsSimulatorPackage.JointDriveMode;
import jp.go.aist.hrp.simulator.DynamicsSimulatorPackage.LinkDataType;
import jp.go.aist.hrp.simulator.DynamicsSimulatorPackage.SensorOption;

import com.generalrobotix.ui.GrxBaseItem;
import com.generalrobotix.ui.GrxPluginManager;
import com.generalrobotix.ui.item.GrxModelItem;
import com.generalrobotix.ui.util.Grx3DViewClickListener;
import com.generalrobotix.ui.util.GrxCorbaUtil;
import com.generalrobotix.ui.util.GrxDebugUtil;
import com.generalrobotix.ui.view.Grx3DView;
import com.sun.j3d.utils.picking.PickCanvas;
import com.sun.j3d.utils.picking.PickTool;

/**
 * ビヘイビア管理のための窓口。
 * モードの切替えをIseBehaviorクラス、IseBehaviorHandlerクラスに伝える。
 * ハンドラクラスからイベントを受け取るリスナの役割をもつ。
 */
public class BehaviorManager implements WorldReplaceListener {
    //--------------------------------------------------------------------
    // 定数
    // オペレーションモード
    public static final int OPERATION_MODE_NONE         = 0;
    public static final int OBJECT_ROTATION_MODE        = 1;
    public static final int OBJECT_TRANSLATION_MODE     = 2;
    public static final int JOINT_ROTATION_MODE         = 3;
    public static final int FITTING_FROM_MODE           = 4;
    public static final int FITTING_TO_MODE             = 5;
    public static final int INV_KINEMA_FROM_MODE        = 6;
    public static final int INV_KINEMA_TRANSLATION_MODE = 7;
    public static final int INV_KINEMA_ROTATION_MODE    = 8;

    //  ビューモード
    public static final int WALK_VIEW_MODE     = 1;
    public static final int ROOM_VIEW_MODE     = 2;
    public static final int PARALLEL_VIEW_MODE = 3;

    // インスタンス変数
    private IseBehavior behavior_;
    private IseBehaviorHandler handler_;
    private int viewMode_;
    private int operationMode_;
    private ThreeDDrawable drawable_;
    private BehaviorInfo info_;
    private BehaviorHandler indicator_;
    private GrxPluginManager manager_;
    private Grx3DView viewer_;
    //--------------------------------------------------------------------
    // コンストラクタ
    public BehaviorManager(GrxPluginManager manager) {
    	manager_ = manager;
        //Simulator.getInstance().addWorldReplaceListener(this);
    }

    // 公開メソッド
    public void setThreeDViewer(Grx3DView viewer) {
    	viewer_ = viewer;
        PickCanvas pickCanvas = new PickCanvas(
            viewer.getCanvas3D(),
            viewer.getBranchGroupRoot()
        );
        pickCanvas.setMode(PickTool.GEOMETRY_INTERSECT_INFO);

        handler_ = new IseBehaviorHandler();

        behavior_ = new IseBehavior(handler_);
 
        BranchGroup bg = new BranchGroup();
        bg.addChild(behavior_);

        BranchGroup bgRoot = viewer_.getBranchGroupRoot();
        bgRoot.addChild(bg);
        
        //drawable_ = viewer.getDrawable();
        drawable_ = viewer;

        ViewInfo viewInfo = drawable_.getViewInfo();
        switch (viewInfo.getViewMode()) {
        case ViewInfo.VIEW_MODE_ROOM:
            setViewMode(ROOM_VIEW_MODE);
            break;
        case ViewInfo.VIEW_MODE_WALK:
            setViewMode(WALK_VIEW_MODE);
            break;
        case ViewInfo.VIEW_MODE_PARALLEL:
            setViewMode(PARALLEL_VIEW_MODE);
            break;
        }
        //TransformGroup tgView = drawable_.getTransformGroupRoot();
        info_ = new BehaviorInfo(manager_, pickCanvas, drawable_);

        behavior_.setBehaviorInfo(info_);
    }

    public void setViewIndicator(BehaviorHandler handler) {
        indicator_ = handler;
    }

    public void setViewMode(int mode) {
        viewMode_ = mode;
        handler_.setViewMode(viewMode_);
    }

    public void setOperationMode(int mode) {
        operationMode_ = mode;
        handler_.setOperationMode(operationMode_);
    }

    public void setViewHandlerMode(String str) {
        handler_.setViewHandlerMode(str);
    }

    public boolean fit() {
        return handler_.fit(info_);
    }

    public void setPickTarget(TransformGroup tg) {
        BehaviorInfo info = behavior_.getBehaviorInfo();
        handler_.setPickTarget(tg, info);
    }

    //--------------------------------------------------------------------
    // ViewChangeListenerの実装
    public void viewChanged(ViewChangeEvent evt) {
        Transform3D transform = new Transform3D();
        evt.getTransformGroup().getTransform(transform);

        // データの通知は ViewNode に任せる
        drawable_.setTransform(transform);
    }

    //--------------------------------------------------------------------
    // WorldReplaceListenerの実装
    public void replaceWorld(List<GrxBaseItem> list) {
        //handler_ = new IseBehaviorHandler();
        //behavior_ = new IseBehavior(handler_);
 
        //BranchGroup bg = new BranchGroup();
        //bg.addChild(behavior_);

        //BranchGroup bgRoot = viewer_.getBranchGroupRoot();
        //bgRoot.addChild(bg);

        //InvKinemaResolver resolver = new InvKinemaResolver(world_);
        //ProjectManager project = ProjectManager.getInstance();
        //resolver.setDynamicsSimulator(project.getDynamicsSimulator());
        //handler_.setInvKinemaResolver(resolver);
        
        if (handler_ != null) 
        	handler_.setViewIndicator(indicator_);
    }

    public boolean updateDynamicsSimulator(boolean force) {
    	if (currentDynamics_ == null || force) {
    		try {
    			if (handler_ != null) {
    				initDynamicsSimulator();
    				InvKinemaResolver resolver = new InvKinemaResolver(manager_);
    				resolver.setDynamicsSimulator(currentDynamics_);
    				handler_.setInvKinemaResolver(resolver);
    			}
    		} catch (Exception e) {
    			currentDynamics_ = null;
    			e.printStackTrace();
    			return false;
    		}
    	}
    	return true;
    }
    DynamicsSimulator currentDynamics_;
	DynamicsSimulator getDynamicsSimulator(boolean update) {
		//currentDynamics_ = dynamicsMap_.get(currentWorld_);
		if (update && currentDynamics_ != null) {
            try {
			  currentDynamics_.destroy();
            } catch (Exception e) {
				GrxDebugUtil.printErr("getDynamicsSimulator: destroy failed.");
            }
			currentDynamics_ = null;
		}
		
		if (currentDynamics_ == null) {
			try {
				org.omg.CORBA.Object obj = //process_.get(DynamicsSimulatorID_).getReference();
				GrxCorbaUtil.getReference("DynamicsSimulatorFactory",
						"localhost", 2809);
				DynamicsSimulatorFactory ifactory = DynamicsSimulatorFactoryHelper
						.narrow(obj);
				currentDynamics_ = ifactory.create();
				currentDynamics_._non_existent();
				//dynamicsMap_.put(currentWorld_, currentDynamics_);

			} catch (Exception e) {
				GrxDebugUtil.printErr("getDynamicsSimulator: create failed.");
				currentDynamics_ = null;
			}
		}
		return currentDynamics_;
	}

	public boolean initDynamicsSimulator() {
		getDynamicsSimulator(true);

		try {
			List modelList = manager_.getSelectedItemList(GrxModelItem.class);
			for (int i=0; i<modelList.size(); i++) {
				GrxModelItem model = (GrxModelItem)modelList.get(i);
				if (model.cInfo_ != null)
					currentDynamics_.registerCharacter(model.getName(), model.cInfo_);
			}

			IntegrateMethod m = IntegrateMethod.EULER;
			currentDynamics_.init(0.005, m, SensorOption.ENABLE_SENSOR);
			currentDynamics_.setGVector(new double[] { 0.0, 0.0, 9.8});
			
			for (int i=0; i<modelList.size(); i++) {
				GrxModelItem model = (GrxModelItem) modelList.get(i);
				if (model.lInfo_ == null)
					continue;
				
				String base = model.lInfo_[0].name;
				currentDynamics_.setCharacterLinkData(
					model.getName(), base, LinkDataType.ABS_TRANSFORM, 
					model.getTransformArray(base));
				
				currentDynamics_.setCharacterAllJointModes(
					model.getName(), JointDriveMode.TORQUE_MODE);
					
				currentDynamics_.setCharacterAllLinkData(
					model.getName(), LinkDataType.JOINT_VALUE, 
					model.getJointValues());
			}
			
			/*
			  double[] K = new double[] { 100000, 100000, 100000, 800, 800, 800 };
			  double[] C = new double[] { 6000, 6000, 6000, 5, 5, 5 };
			  List<GrxBaseItem> collisionPair = manager_
					.getSelectedItemList(GrxCollisionPairItem.class);
			it = collisionPair.iterator();
			while (it.hasNext()) {
				GrxCollisionPairItem item = (GrxCollisionPairItem) it.next();
				currentDynamics_.registerCollisionCheckPair(
						item.getProperty("objectName1",""), 
						item.getProperty("jointName1",""), 
						item.getProperty("objectName2",""), 
						item.getProperty("jointName2",""), 0.5, 0.5, K, C);
			}*/
            //state_.value = null;
		} catch (Exception e) {
			GrxDebugUtil.printErr("initDynamicsSimulator:", e);
			return false;
		}
		return true;
	}

	public void addClickListener( Grx3DViewClickListener listener ){
		behavior_.addClickListener( listener );
	}
	public void removeClickListener( Grx3DViewClickListener listener ){
		behavior_.removeClickListener( listener );
	}

}